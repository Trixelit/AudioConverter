#include <iostream>
#include <string>
#include "converter.h"


int main(int argc, char** argv) {

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

    return convertAudio(inputPath, outputPath, target);
}