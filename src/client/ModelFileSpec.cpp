//
// Created by Chris Davey on 25/10/2026.
//

#include "include/ModelFileSpec.h"

#include <fstream>
#include <ranges>
#include <algorithm>
#include <istream>
#include <sstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sdrcomms {
    ModelFileSpec::ModelFileSpec(std::string name,
                                 std::string version,
                                 int class_map_version,
                                 std::string onnx_file,
                                 bool logits,
                                 int sample_rate_hz,
                                 int input_length,
                                 std::vector<std::string> input_transforms) {
        this->name = name;
        this->version = version;
        this->class_map_version = class_map_version;
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
        json jobj = json::parse(data);
        std::string name = jobj["name"].get<std::string>();
        std::string version = jobj["version"].get<std::string>();
        int class_map_version = jobj["class_map_version"].get<int>();
        std::string onnx_file = jobj["onnx_file"].get<std::string>();
        int sample_rate_hz = jobj["sample_rate_hz"].get<int>();
        bool logits = jobj["logits"].get<bool>();
        int input_length = jobj["input_length"].get<int>();
        auto transforms = jobj["input_transforms"].get<std::vector<std::string> >();
        return ModelFileSpec{name, version, class_map_version, onnx_file, logits, sample_rate_hz, input_length, transforms};
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
        return ModelFileSpec::from_json(json_data);
    }

    /**
         * get all transforms in upper case
         * @return
         */
    std::vector<std::string> ModelFileSpec::getTransforms() const {
        auto to_upper = [](std::string str) {
            std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
                return std::toupper(c);
            });
            return str;
        };
        auto upper_transforms = input_transforms
                                | std::views::transform(to_upper)
                                | std::ranges::to<std::vector<std::string>>();
        return upper_transforms;

    }
}
