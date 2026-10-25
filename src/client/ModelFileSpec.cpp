//
// Created by Chris Davey on 25/10/2026.
//

#include "include/ModelFileSpec.h"

#include <fstream>
#include <istream>

namespace sdrcomms {
    ModelFileSpec::ModelFileSpec(std::string name,
                                 std::string version,
                                 std::string onnx_file,
                                 bool logits,
                                 int sample_rate_hz,
                                 int input_length,
                                 std::vector<std::string> input_transforms) {
        this->name = name;
        this->version = version;
        this->onnx_file = onnx_file;
        this->logits = logits;
        this->sample_rate_hz = sample_rate_hz;
        this->input_length = input_length;
        this->input_transforms = input_transforms;
    }

    /**
     * Parse from json string in format:
```
{
  "name": "mod_cbam",
  "version": "002D_export",
  "onnx_file": "mod_cbam/002D_export/2026-10-24/model.onnx",
  "sample_rate_Hz": 6000,
  "input_length": 2048,
  "input_transforms": ["iq", "fft"]
}
```
     * @param data
     * @return
     */
    ModelFileSpec ModelFileSpec::from_json(const std::string &data) {
        nlohmann::json json = nlohmann::json::parse(data);
        std::string name = json["name"].get<std::string>();
        std::string version = json["version"].get<std::string>();
        std::string onnx_file = json["onnx_file"].get<std::string>();
        int sample_rate_hz = json["sample_rate_hz"].get<int>();
        bool logits = json["logits"].get<bool>();
        int input_length = json["input_length"].get<int>();
        auto transforms = json["input_transforms"].get<std::vector<std::string> >();
        return ModelFileSpec{name, version, onnx_file, logits, sample_rate_hz, input_length, transforms};
    }


    /**
     * read the model filespec from file.
     * @param filename
     * @return
     */
    ModelFileSpec ModelFileSpec::from_file(const std::string& filename) {
        std::ifstream in { filename };
        if (!in.is_open()) {
            return {};
        }
        std::stringstream buffer {};
        buffer << in.rdbuf();
        std::string json_data = buffer.str();
        in.close();
        return from_json(json_data);
    }
}
