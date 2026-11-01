//
// Created by Chris Davey on 1/11/2026.
//

#include "include/RootMeanSquareProcessor.h"

#include <algorithm>
#include <iostream>
#include <numeric>

namespace sdrcomms {
    RootMeanSquareProcessor::RootMeanSquareProcessor(std::string attach_key, std::string process_key, std::shared_ptr<SampleProcessor> inner) {
        this->attach_key = attach_key;
        this->process_key = process_key;
        this->inner = inner;
    }

    /**
         * process the supplied IQBlock
         * @param block
         */
    void RootMeanSquareProcessor::process(IQBlock &block,
                                          std::map<std::string, std::shared_ptr<IQBlock> > &attachments) {
        if (!process_key.empty()) {
            std::shared_ptr<IQBlock> innerBlock = get_attachment(attachments);
            if (innerBlock != nullptr) {
                process_samples(*innerBlock, attachments);
            }
        } else {
            process_samples(block, attachments);
        }

        if (this->inner != nullptr) {
            // in this case the input block will be passed directly into the next processor
            this->inner->process(block, attachments);
        }
    }



    std::shared_ptr<IQBlock> RootMeanSquareProcessor::get_attachment(std::map<std::string, std::shared_ptr<IQBlock>> &attachments) {
        if (process_key.empty()) {
            return nullptr;
        }
        if (attachments.find(process_key) == attachments.end()) {
            return nullptr;
        }
        return attachments[process_key];
    }

    void RootMeanSquareProcessor::process_samples(IQBlock &block, std::map<std::string, std::shared_ptr<IQBlock>> &attachments) {
        size_t n = block.samples.size();
        std::vector<double> abs_sq_values(n);
        std::ranges::generate(abs_sq_values, [block, i = 0]() mutable {
            // abs * abs
            double value = static_cast<double>(std::norm(block.samples[i]));
            i++;
            return value;
        });
        double sum_square = std::reduce(abs_sq_values.begin(), abs_sq_values.end());
        double rms = std::sqrt(sum_square / n);
        double rms_db = 20.0 * std::log10(rms);
        std::shared_ptr<IQBlock> rms_block = std::make_shared<IQBlock>(
            block.channel,
            block.timestampMs,
            block.sample_size,
            std::vector<std::complex<float> >{rms_db, 0.0}
        );

        std::cout << "RMS: " << rms_db << " dB" << std::endl;

        rms_block->centerFreq = block.centerFreq;
        // the rms measure is attached as information associated with the block
        attachments[attach_key] = rms_block;
    }
}
