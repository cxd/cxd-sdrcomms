//
// Created by Chris Davey on 31/10/2026.
//

#include "include/ClassMap.h"

#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sdrcomms {


    ClassMap::ClassMap(int version,
        int numClasses,
        std::map<std::string, int>& classMap) {
        this->version = version;
        this->numClasses = numClasses;
        this->classMap = classMap;
    }

    /**
        * load from json string
        * @param data
        * @return
        */
    ClassMap ClassMap::from_json(const std::string& data) {
        json jobj = json::parse(data);
        int version = jobj["version"];
        int numClasses = jobj["num_classes"];
        auto classMap = jobj["classes"].get<std::map<std::string, int>>();
        return ClassMap(version, numClasses, classMap);
    }

    /**
     * load from filename.
     * @param filename
     * @return
     */
    ClassMap ClassMap::from_file(const std::string& filename) {
        std::ifstream in { filename };
        if (!in.is_open()) {
            return {};
        }
        std::stringstream buffer {};
        buffer << in.rdbuf();
        std::string json_data = buffer.str();
        in.close();
        return ClassMap::from_json(json_data);
    }
} // sdrcomms