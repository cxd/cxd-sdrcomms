//
// Created by Chris Davey on 22/11/2026.
//

#include "include/SlidingCorrelationMetricProcessor.h"

#include <iostream>

namespace sdrcomms {

    SlidingCorrelationMetricProcessor::SlidingCorrelationMetricProcessor(std::string attach_key, std::string process_key, std::shared_ptr<SampleProcessor> inner) {
        this->attach_key = attach_key;
        this->process_key = process_key;
        this->inner = inner;
    }

    /**
     * compute sliding correlation metric
     * @param block
     * @param attachments
     */
    void SlidingCorrelationMetricProcessor::process(IQBlock &block, std::map<std::string, std::shared_ptr<IQBlock>> &attachments) {
        auto source = std::make_shared<IQBlock>( block );
        if (!process_key.empty()) {
            source = get_attachment(process_key, attachments);
        }
        if (source == nullptr) {
            return;
        }
        float metric = compute_metric(source);
        float flag = 0.0;
        if (metric > threshold) {
            flag = 1.0;
        }
        std::shared_ptr<IQBlock> metric_block = std::make_shared<IQBlock>(
            block.channel,
            block.timestampMs,
            block.sample_size,
            std::vector<std::complex<float> >{metric, flag}
        );

        std::cout << "Corr Metric: " << metric << std::endl;

        metric_block->centerFreq = block.centerFreq;
        // the rms measure is attached as information associated with the block
        attachments[attach_key] = metric_block;

        if (inner != nullptr) {
            inner->process(block, attachments);
        }

    }


    /**
     * check if the trigger is activated
     * @param bool
     */
    bool SlidingCorrelationMetricProcessor::is_active(std::map<std::string, std::shared_ptr<IQBlock>> &attachments) {
        auto metricBlock = get_attachment(attach_key, attachments);
        if (metricBlock == nullptr) {
            return false;
        }
        if (metricBlock->samples.size() == 0) {
            return false;
        }
        float metric = metricBlock->samples[0].real();
        return metric > threshold;
    }


    /**
     * compute sliding sum of complex conjugate products
     * @param source
     * @return
     */
    float SlidingCorrelationMetricProcessor::compute_metric(const std::shared_ptr<IQBlock>& source) {
        if (source->samples.size() == 0) {
            return 0.0f;
        }

        float metric = 0.0f;

        for (size_t i=0; i<source->samples.size(); i++) {
            if (i+delay > source->samples.size()) {
                break;
            }
            float product = 0.0f;
            std::complex<float> product_cmplx {0.0f, 0.0f };
            float energy = 0.0f;

            size_t size = std::min(i+average_window, source->sample_size);
            for (size_t j=i+delay; j<size;j++) {
                auto sample = source->samples[j];
                auto delaySample = source->samples[j-delay];
                auto conjugate = std::conj(delaySample);
                product_cmplx += sample * conjugate;
                energy += std::norm(sample);
            }
            // we only need to detect the first window that may contain a peak ratio
            product = std::norm(product_cmplx);
            if (energy > 0) {
                metric = product / (energy*energy + 1e-12f);
            }
            if (metric > threshold) {
                return metric;
            }
        }

        return metric;
    }

} // sdrcomms