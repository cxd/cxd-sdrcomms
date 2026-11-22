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
        virtual void process(IQBlock &block, std::map<std::string, std::shared_ptr<IQBlock>> &attachments) = 0;

        /**
         * each part of the processor chain may be able to teardown
         */
        virtual void teardown() {
            // override this method to perform any necessary cleanup
            // but call the base method first
            if (inner != nullptr) {
                inner->teardown();
            }
        }

    protected:
        std::string attach_key;
        std::string process_key = "";
        /**
         * pointer to inner sample processor
         */
        std::shared_ptr<SampleProcessor> inner;

        /**
         * forward attachments along the chain to the inner block
         * this passes the attachments from the parent block to the inner block
         * so they can be used by the next processor in the chain
         * @param block
         * @param attach_key
         * @param innerBlock
         */
        void forwardAttachments(IQBlock &block, std::string attach_key, std::shared_ptr<IQBlock> innerBlock) {
            collectAttachments(block);
            auto attachments = block.attachments;
            innerBlock->attachments = attachments;
        }

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


        std::shared_ptr<IQBlock> get_attachment(const std::string& key, std::map<std::string, std::shared_ptr<IQBlock>> &attachments) {
            if (key.empty()) {
                return nullptr;
            }
            if (attachments.find(key) == attachments.end()) {
                return nullptr;
            }
            return attachments[key];
        }


    private:
        /**
         * copy attachments into the supplied map.
         * traverse referenced IQ block pointers.
         * @param accum
         * @param block
         */
        void foldAttachments(std::map<std::string, std::shared_ptr<IQBlock>> &accum, const std::shared_ptr<IQBlock> &block) {
            if (block == nullptr) {
                return;
            }
            for (auto &[key, value] : block->attachments) {
                if (accum.contains(key)) {
                    continue;
                }
                accum.insert_or_assign(key, value);
                if (value->attachments.size() > 0) {
                    foldAttachments(accum, value);
                }
            }
            block->attachments.clear();
        }

    };

    /**
     * a trigger processor is a special type of sample processor
     * that can be used to make a decision such as thresholding a metric
     */
    class TriggerProcessor  : public SampleProcessor {
    public:

        /**
         * check if the trigger is activated
         * @param bool
         */
        virtual bool is_active(std::map<std::string, std::shared_ptr<IQBlock>> &attachments) = 0;
    };
}


#endif //SDRCOMMS_SAMPLEPROCESSOR_H
