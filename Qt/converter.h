//
// Created by Trixie on 12/02/2026.
//

#ifndef AUDIOCONVERTER_CONVERTER_H
#define AUDIOCONVERTER_CONVERTER_H

#include <string>
#include <functional>

enum class Target {
    TARGET_YEASTAR,
    TARGET_3CX
};

double getAudioDurationSeconds(const std::string& path);
int convertAudio(const std::string& inPath, const std::string& outPath, Target target, std::function<void(double)> progressCallback = nullptr);
std::string ffmpeg_err(int err);

#endif //AUDIOCONVERTER_CONVERTER_H
