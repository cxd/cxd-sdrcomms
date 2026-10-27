//
// Created by Chris Davey on 27/10/2026.
//

#ifndef SDRCOMMS_PROCESSOROPS_H
#define SDRCOMMS_PROCESSOROPS_H
#include <algorithm>
#include <string>

namespace sdrcomms {
    enum class ProcessorOps {
        NONE = 0,
        DOWNSAMPLE = 1,
        FFT = 2
    };

    constexpr std::string to_lowercase(std::string text) {
        std::transform(text.begin(), text.end(), text.begin(), ::tolower);
        return text;
    }

    constexpr ProcessorOps operation_from_string(std::string label) {
        label = to_lowercase(label);
        if (label == "none") {
            return ProcessorOps::NONE;
        }
        if (label == "downsample" || label == "down_sample" || label == "resample") {
            return ProcessorOps::DOWNSAMPLE;
        }
        if (label == "fft") {
            return ProcessorOps::FFT;
        }
        return ProcessorOps::NONE;
    }

    constexpr std::string operation_to_string(const ProcessorOps& op) {
        switch (op) {
            case ProcessorOps::NONE:
                return "NONE";
            case ProcessorOps::DOWNSAMPLE:
                return "DOWNSAMPLE";
            case ProcessorOps::FFT:
                return "FFT";
        }
    }
}

#endif //SDRCOMMS_PROCESSOROPS_H
