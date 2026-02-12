//
// Created by Trixie on 12/02/2026.
//
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
#include "converter.h"

std::string ffmpeg_err(int err) {
    char buf[AV_ERROR_MAX_STRING_SIZE];
    av_strerror(err, buf, sizeof(buf));
    return std::string(buf);
}

int convertAudio(const std::string& inPath, const std::string& outPath, Target target) {

    const char* inputPath = inPath.c_str();
    const char* outputPath = outPath.c_str();

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

    if (decoderContext->ch_layout.nb_channels == 0) {
        av_channel_layout_default(
            &decoderContext->ch_layout,
            audioStream->codecpar->ch_layout.nb_channels
        );
    }

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

            outFrame->nb_samples = inFrame->nb_samples;
            av_frame_get_buffer(outFrame, 0);

            int out_samples = swr_convert(
                swr,
                outFrame->data,
                outFrame->nb_samples,
                (const uint8_t**)inFrame->data,
                inFrame->nb_samples
            );

            if (out_samples < 0) {
                std::cerr << "swr_convert failed: " << ffmpeg_err(out_samples) << "\n";
                break;
            }

            outFrame->nb_samples = out_samples;

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

    avcodec_free_context(&decoderContext);
    avcodec_free_context(&encoderContext);
    swr_free(&swr);
    av_frame_free(&inFrame);
    av_frame_free(&outFrame);
    avformat_free_context(outFmt);
    avformat_close_input(&fmt);
    return 0;
}