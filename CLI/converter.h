//
// Created by Trixie on 12/02/2026.
//

#ifndef AUDIOCONVERTER_CONVERTER_H
#define AUDIOCONVERTER_CONVERTER_H

#include <string>

enum class Target {
    TARGET_YEASTAR,
    TARGET_3CX
};

int convertAudio(const std::string& inPath, const std::string& outPath, Target target);
std::string ffmpeg_err(int err);

#endif //AUDIOCONVERTER_CONVERTER_H