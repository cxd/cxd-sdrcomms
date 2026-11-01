//
// Created by Chris Davey on 1/11/2026.
//

#ifndef SDRCOMMS_ROOTMEANSQUAREPROCESSOR_H
#define SDRCOMMS_ROOTMEANSQUAREPROCESSOR_H
#include "IQBlock.h"
#include <map>
#include <memory>
#include "SampleProcessor.h"

namespace sdrcomms {
    class RootMeanSquareProcessor : public SampleProcessor {
    public:
        /**
         *
         * @param attach_key
         * @param process_key
         * the key to use to extract samples for processing from the attachments map
         * if left empty it will process the block samples instead.
         * @param inner
         */
        RootMeanSquareProcessor(std::string attach_key, std::string process_key, std::shared_ptr<SampleProcessor> inner);

        /**
             * process the supplied IQBlock
             * @param block
             */
        void process(IQBlock &block, std::map<std::string, std::shared_ptr<IQBlock>> &attachments) override;

    private:

        std::string process_key = "";

        std::shared_ptr<IQBlock> get_attachment(std::map<std::string, std::shared_ptr<IQBlock>> &attachments);

        void process_samples(IQBlock &block, std::map<std::string, std::shared_ptr<IQBlock>> &attachments);

    };
}


#endif //SDRCOMMS_ROOTMEANSQUAREPROCESSOR_H
