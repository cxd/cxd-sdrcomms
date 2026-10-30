//
// Created by Chris Davey on 22/9/2026.
//

#include "MockProcessor.h"
#include <iostream>

MockProcessor::MockProcessor() {
    last = std::chrono::steady_clock::now();
}

void MockProcessor::process(sdrcomms::IQBlock &block, std::map<std::string, std::shared_ptr<sdrcomms::IQBlock>> &attachments) {
    auto now = std::chrono::steady_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->last).count();
    int sz = block.samples.size();
    //std::cout << "[" << now.time_since_epoch() << "] " << delta << " ms " << sz << " blocks" << std::endl;
    this->last = now;
}
