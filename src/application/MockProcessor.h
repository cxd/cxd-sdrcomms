//
// Created by Chris Davey on 22/9/2026.
//

#ifndef SDRCOMMS_MOCKPROCESSOR_H
#define SDRCOMMS_MOCKPROCESSOR_H
#include "SampleProcessor.h"
#include "IQBlock.h"
#include <chrono>

class MockProcessor : public sdrcomms::SampleProcessor {
public:
    MockProcessor();

    void process(sdrcomms::IQBlock &block, std::map<std::string, std::shared_ptr<sdrcomms::IQBlock>> &attachments) override;

private:
    std::chrono::steady_clock::time_point last;
};


#endif //SDRCOMMS_MOCKPROCESSOR_H
