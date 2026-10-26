//
// Created by Chris Davey on 22/9/2026.
//

#ifndef SDRCOMMS_SAMPLEPROCESSOR_H
#define SDRCOMMS_SAMPLEPROCESSOR_H
#include "IQBlock.h"

namespace sdrcomms {
    class SampleProcessor {
    public:
        /**
         * process the supplied IQBlock
         * @param block
         */
        virtual void process(IQBlock &block) = 0;

    protected:
        std::string attach_key;

        /**
         * walk the attachments of the supplied block and lift them into the current top level block.
         * This can be performed at the end of the processing chain
         * by a parent processor in the hierarchy.
         * This collects all attachments into a map and then inserts them into the parent block.
         * @param block
         */
        void collectAttachments(IQBlock &block) {
            std::map<std::string, std::shared_ptr<IQBlock>> accum;
            for (auto &[key, value] : block.attachments) {
                foldAttachments(accum, value);
            }
            for (auto &[key, value] : accum) {
                block.attachments.insert_or_assign(key, value);
            }
        }

    private:
        /**
         * copy attachments into the supplied map.
         * traverse referenced IQ block pointers.
         * @param accum
         * @param block
         */
        void foldAttachments(std::map<std::string, std::shared_ptr<IQBlock>> &accum, const std::shared_ptr<IQBlock> &block) {
            for (auto &[key, value] : block->attachments) {
                accum.insert_or_assign(key, value);
                if (value->attachments.size() > 0) {
                    foldAttachments(accum, value);
                }
            }
            block->attachments.clear();
        }

    };
}


#endif //SDRCOMMS_SAMPLEPROCESSOR_H
