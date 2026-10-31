//
// Created by Chris Davey on 31/10/2026.
//

#ifndef SDRCOMMS_CLASSMAP_H
#define SDRCOMMS_CLASSMAP_H
#include <map>
#include <string>

namespace sdrcomms {
    class ClassMap {
    public:
        ClassMap() = default;

        ClassMap(int version,
            int numClasses,
            std::map<std::string, int>& classMap);

        /**
         * the class map version.
         */
        int version;

        /**
         * the number of classes in the map.
         */
        int numClasses;

        /**
         * the mapping between class names and indices.
         */
        std::map<std::string, int> classMap;

        /**
         * load from json string
         * @param data
         * @return
         */
        static ClassMap from_json(const std::string& data);

        /**
         * load from filename.
         * @param filename
         * @return
         */
        static ClassMap from_file(const std::string& filename);

    };
} // sdrcomms

#endif //SDRCOMMS_CLASSMAP_H
