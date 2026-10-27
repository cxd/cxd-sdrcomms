//
// Created by Chris Davey on 25/10/2026.
//

#ifndef SDRCOMMS_MODELFILESPEC_H
#define SDRCOMMS_MODELFILESPEC_H
#include "nlohmann/json_fwd.hpp"

/**
 * ModeFileSpec
 *
 * represents the file specification for a model stored in json.
* {
  "name": "mod_cbam",
  "version": "002D_export",
  "onnx_file": "mod_cbam/002D_export/2026-10-24/model.onnx",
  "sample_rate_Hz": 6000,
  "input_length": 2048,
  "input_transforms": ["iq", "fft"]
}
 */
namespace sdrcomms {
    class ModelFileSpec {
    public:
        ModelFileSpec() = default;

        ModelFileSpec(std::string name,
            std::string version,
            std::string onnx_file,
            bool logits,
            int sample_rate_hz,
            int input_length,
            std::vector<std::string> input_transforms);

        /**
         * load from json string
         * @param data
         * @return
         */
        static ModelFileSpec from_json(const std::string& data);

        /**
         * load from filename.
         * @param filename
         * @return
         */
        static ModelFileSpec from_file(const std::string& filename);


        std::string name;
        std::string version;
        std::string onnx_file;
        bool logits;
        int sample_rate_hz;
        int input_length;
        std::vector<std::string> input_transforms;

        /**
         * get all transforms in upper case
         * @return
         */
        std::vector<std::string> getTransforms() const ;

    };
}

#endif //SDRCOMMS_MODELFILESPEC_H
