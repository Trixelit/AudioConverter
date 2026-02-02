#include <iostream>
#include <string>
#include <windows.h>
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/error.h>
}

std::string ffmpeg_err(int err) {
    char buf[AV_ERROR_MAX_STRING_SIZE];
    av_strerror(err, buf, sizeof(buf));
    return std::string(buf);
}


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

    avformat_close_input(&fmt);
    return 0;
}