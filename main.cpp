#include <iostream>
#include <chrono>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <cmath>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}

const char* ASCII = " .,:;i1tfLCG08@";

// 获取终端宽度
int get_terminal_width() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

// 获取终端高度
int get_terminal_height() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_row;
}

// 设置终端为原始模式
void set_terminal_raw() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

int main() {
    // 初始化 FFmpeg
    avformat_network_init();

    // 打开视频文件
    AVFormatContext* format_ctx = avformat_alloc_context();
    if (avformat_open_input(&format_ctx, "in.mp4", NULL, NULL) != 0) {
        std::cerr << "无法打开文件" << std::endl;
        return 1;
    }

    // 获取流信息
    if (avformat_find_stream_info(format_ctx, NULL) < 0) {
        std::cerr << "找不到流信息" << std::endl;
        return 1;
    }

    // 查找视频流
    int video_stream_index = -1;
    const AVCodec* codec = nullptr;
    AVCodecParameters* codec_params = nullptr;
    for (int i = 0; i < format_ctx->nb_streams; i++) {
        if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            codec_params = format_ctx->streams[i]->codecpar;
            codec = avcodec_find_decoder(codec_params->codec_id);
            break;
        }
    }

    if (video_stream_index == -1 || !codec) {
        std::cerr << "找不到视频流或编解码器" << std::endl;
        return 1;
    }

    // 创建编解码器上下文
    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codec_ctx, codec_params);
    if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
        std::cerr << "无法打开编解码器" << std::endl;
        return 1;
    }

    // 获取终端尺寸
    int term_w = get_terminal_width();
    int term_h = get_terminal_height();

    // 创建缩放上下文
    SwsContext* sws_ctx = sws_getContext(
        codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
        term_w, term_h, AV_PIX_FMT_GRAY8,
        SWS_BILINEAR, NULL, NULL, NULL
    );

    if (!sws_ctx) {
        std::cerr << "无法创建缩放上下文" << std::endl;
        return 1;
    }

    // 分配帧和包
    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    AVFrame* gray_frame = av_frame_alloc();
    gray_frame->format = AV_PIX_FMT_GRAY8;
    gray_frame->width = term_w;
    gray_frame->height = term_h;
    av_frame_get_buffer(gray_frame, 0);

    // 设置终端为原始模式并隐藏光标
    set_terminal_raw();
    std::cout << "\033[?25l"; // 隐藏光标

    // 获取视频帧率
    double frame_rate = av_q2d(format_ctx->streams[video_stream_index]->avg_frame_rate);
    if (frame_rate <= 0.0) {
        frame_rate = 30.0; // 默认30fps
    }
    const double frame_delay = 1'000'000 / frame_rate; // 每帧延迟（微秒）

    // 主循环：读取并显示视频帧
    auto start_time = std::chrono::high_resolution_clock::now();
    while (av_read_frame(format_ctx, packet) >= 0) {
        if (packet->stream_index == video_stream_index) {
            avcodec_send_packet(codec_ctx, packet);
            if (avcodec_receive_frame(codec_ctx, frame) == 0) {
                // 缩放帧到终端尺寸
                sws_scale(sws_ctx, 
                    frame->data, frame->linesize, 0, codec_ctx->height,
                    gray_frame->data, gray_frame->linesize
                );

                // 移动光标到左上角并输出 ASCII 帧
                std::cout << "\033[H";
                for (int y = 0; y < term_h; y++) {
                    for (int x = 0; x < term_w; x++) {
                        uint8_t lum = gray_frame->data[0][y * gray_frame->linesize[0] + x];
                        int index = static_cast<int>((lum / 255.0) * (strlen(ASCII) - 1));
                        std::cout << ASCII[index];
                    }
                    std::cout << std::endl;
                }

                // 精确帧率控制
                auto end_time = std::chrono::high_resolution_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
                auto remaining = std::chrono::microseconds(static_cast<int>(frame_delay)) - elapsed;

                if (remaining.count() > 0) {
                    usleep(remaining.count());
                }
                start_time = std::chrono::high_resolution_clock::now();
            }
        }
        av_packet_unref(packet);
    }

    // 恢复终端设置并显示光标
    std::cout << "\033[?25h";
    std::cout << "\033[0m"; // 重置终端颜色

    // 释放资源
    av_frame_free(&frame);
    av_frame_free(&gray_frame);
    av_packet_free(&packet);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&format_ctx);
    sws_freeContext(sws_ctx);

    return 0;
}