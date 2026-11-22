//
// Created by Chris Davey on 22/11/2026.
//

#ifndef SDRCOMMS_SLIDINGCORRELATIONMETRICPROCESSOR_H
#define SDRCOMMS_SLIDINGCORRELATIONMETRICPROCESSOR_H
#include "SampleProcessor.h"
#include <memory>
#include <string>

namespace sdrcomms {
    class SlidingCorrelationMetricProcessor : public TriggerProcessor  {

    public:

        SlidingCorrelationMetricProcessor(std::string attach_key, std::string process_key, std::shared_ptr<SampleProcessor> inner);

        /**
         * compute sliding correlation metric
         * @param block
         * @param attachments
         */
        void process(IQBlock &block, std::map<std::string, std::shared_ptr<IQBlock>> &attachments) override;

        /**
         * check if the trigger is activated
         * @param bool
         */
        bool is_active(std::map<std::string, std::shared_ptr<IQBlock>> &attachments) override;

    private:

        int delay = 64;

        int average_window = 256;

        /**
         * the threshold used to detect peaks.
         *
         */
        float threshold = 0.5f;

        /**
         * compute sliding sum of complex conjugate products
         * we only need to detect whether at least one peak is above the threshold
         * in any sequence. If this is the case we can return the metric early.
         * @param source
         * @return
         */
        float compute_metric(const std::shared_ptr<IQBlock>& source);


    };
} // sdrcomms

#endif //SDRCOMMS_SLIDINGCORRELATIONMETRICPROCESSOR_H
