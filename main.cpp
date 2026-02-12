#include <iostream>
#include <string>
#include <windows.h>
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}

std::string ffmpeg_err(int err) {
    char buf[AV_ERROR_MAX_STRING_SIZE];
    av_strerror(err, buf, sizeof(buf));
    return std::string(buf);
}

enum class Target {
    TARGET_YEASTAR,
    TARGET_3CX
};

/*int main(int argc, char** argv) {
    std::cout << "HELLO FROM MAIN\n";
    std::cout << "argc = " << argc << "\n";

    for (int i = 0; i < argc; i++) {
        std::cout << "argv[" << i << "] = " << argv[i] << "\n";
    }

    std::cout << "Sleeping for 5 seconds...\n";
    Sleep(5000);

    return 0;
}*/

int main(int argc, char** argv) {
    std::cout << "argc = " << argc << "\n";
    for (int i = 0; i < argc; i++) {
        std::cout << "argv[" << i << "] = " << argv[i] << "\n";
    }

    if (argc < 4) {
        std::cerr << "Usage: audioconvert <infile> <outfile> --target yeastar|3cx\n";
        return 1;
    }



    const char* inputPath = argv[1];
    const char* outputPath = argv[2];

    Target target;

    if (std::string(argv[3]) != "--target") {
        std::cerr << "Missing --target parameter\n";
        return 1;
    }

    std::string targetStr = argv[4];

    if (targetStr == "yeastar") {
        target = Target::TARGET_YEASTAR;
    } else if (targetStr == "3cx") {
        target = Target::TARGET_3CX;
    } else {
        std::cerr << "Unknown target parameter: " << targetStr << "\n";
        return 1;
    }

    AVFormatContext* fmt = nullptr;

    int ret = avformat_open_input(&fmt, inputPath, nullptr, nullptr);
    if (ret < 0) {
        std::cerr << "Failed to open input: " << ffmpeg_err(ret) << "\n";
        return 1;
    }

    ret = avformat_find_stream_info(fmt, nullptr);
    if (ret < 0) {
        std::cerr << "Failed to find stream info: " << ffmpeg_err(ret) << "\n";
        avformat_close_input(&fmt);
        return 1;
    }

    std::cout << "Opened file successfully!\n";
    std::cout << "Format: " << fmt->iformat->name << "\n";
    std::cout << "Streams: " << fmt->nb_streams << "\n";

    int audioStreamIndex = -1;

    for (unsigned i = 0; i< fmt->nb_streams; i++) {
        if (fmt->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = i;
            break;
        }
    }

    if (audioStreamIndex < 0) {
        std::cerr << "No audio stream found!\n";
        return 1;
    }

    AVStream* audioStream = fmt->streams[audioStreamIndex];
    AVCodecParameters* codecpar = audioStream->codecpar;

    const AVCodec* decoder = avcodec_find_decoder(codecpar->codec_id);
    if (!decoder) {
        std::cerr << "Unsupported codec type: " << codecpar->codec_type << "\n";
        return 1;
    }

    AVCodecContext* decoderContext = avcodec_alloc_context3(decoder);
    avcodec_parameters_to_context(decoderContext, codecpar);

    if (avcodec_open2(decoderContext, decoder, nullptr) < 0) {
        std::cerr << "Failed to open decoder context: " << ffmpeg_err(ret) << "\n";
        return 1;
    }

    const int OUT_SAMPLE_RATE = 8000;
    const AVSampleFormat OUT_SAMPLE_FMT = AV_SAMPLE_FMT_S16;
    const AVChannelLayout OUT_CH_LAYOUT = AV_CHANNEL_LAYOUT_MONO;

    SwrContext* swr = swr_alloc();

    av_opt_set_chlayout(swr, "in_chlayout", &decoderContext->ch_layout, 0);
    av_opt_set_int(swr, "in_sample_rate", decoderContext->sample_rate, 0);
    av_opt_set_sample_fmt(swr, "in_sample_fmt", decoderContext->sample_fmt, 0);

    av_opt_set_chlayout(swr, "out_chlayout", &OUT_CH_LAYOUT, 0);
    av_opt_set_int(swr, "out_sample_rate", OUT_SAMPLE_RATE, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", OUT_SAMPLE_FMT, 0);

    swr_init(swr);

    const AVCodec* encoder;

    if (target == Target::TARGET_3CX) {
        encoder = avcodec_find_encoder(AV_CODEC_ID_PCM_S16LE);
    } else {
        encoder = avcodec_find_encoder(AV_CODEC_ID_PCM_ALAW);
    }

    AVCodecContext* encoderContext = avcodec_alloc_context3(encoder);

    encoderContext->sample_rate = OUT_SAMPLE_RATE;
    encoderContext->ch_layout = OUT_CH_LAYOUT;
    encoderContext->sample_fmt = encoder->sample_fmts[0];
    encoderContext->time_base = AVRational{1, OUT_SAMPLE_RATE};

    avcodec_open2(encoderContext, encoder, nullptr);

    AVFormatContext* outFmt = nullptr;
    avformat_alloc_output_context2(&outFmt, nullptr, "wav", outputPath);

    AVStream* outStream = avformat_new_stream(outFmt, nullptr);
    avcodec_parameters_from_context(outStream->codecpar, encoderContext);
    outStream->time_base = AVRational{1, OUT_SAMPLE_RATE};

    avio_open(&outFmt->pb, outputPath, AVIO_FLAG_WRITE);
    avformat_write_header(outFmt, nullptr);

    AVPacket pkt;
    AVFrame* inFrame = av_frame_alloc();
    AVFrame* outFrame = av_frame_alloc();
    int64_t samples_written = 0;

    while (av_read_frame(fmt, &pkt) >= 0) {
        if (pkt.stream_index != audioStreamIndex) {
            av_packet_unref(&pkt);
            continue;
        }

        avcodec_send_packet(decoderContext, &pkt);

        while (avcodec_receive_frame(decoderContext, inFrame) == 0) {
            outFrame->ch_layout = OUT_CH_LAYOUT;
            outFrame->sample_rate = OUT_SAMPLE_RATE;
            outFrame->format = OUT_SAMPLE_FMT;

            outFrame->nb_samples = swr_get_out_samples(swr, inFrame->nb_samples);
            av_frame_get_buffer(outFrame, 0);

            swr_convert_frame(swr, outFrame, inFrame);

            outFrame->pts = samples_written;
            samples_written += outFrame->nb_samples;

            avcodec_send_frame(encoderContext, outFrame);

            AVPacket outPkt;
            av_init_packet(&outPkt);

            while (avcodec_receive_packet(encoderContext, &outPkt) == 0) {
                av_interleaved_write_frame(outFmt, &outPkt);
                av_packet_unref(&outPkt);
            }
            av_frame_unref(inFrame);
            av_frame_unref(outFrame);
        }

        av_packet_unref(&pkt);
    }

    avcodec_send_frame(encoderContext, nullptr);
    while (avcodec_receive_packet(encoderContext, &pkt) == 0) {
        av_interleaved_write_frame(outFmt, &pkt);
        av_packet_unref(&pkt);
    }
    av_write_trailer(outFmt);

    avformat_close_input(&fmt);
    return 0;
}