#include <iostream>
#include "client.h"
#include "constants.h"
#include <memory>
#include <vector>
#include <chrono>
#include "MockProcessor.h"

int main() {
    std::cout << "Initialising SoapySDR" << std::endl;
    sdrcomms::Client client {"tcp://192.168.0.102:11111", "remote", sdrcomms::DEFAULT_QUEUE_SIZE};
    client.PrintSoapySDRInfo();
    bool isConnectd = client.Connect();
    std::cout << "Client connected: " << isConnectd << std::endl;
    if (isConnectd) {
        client.PrintDeviceInfo();
    }
    client.SetCenterFrequency(sdrcomms::DEFAULT_CENTER_FREQUENCY);
    auto processor = std::make_shared<MockProcessor>();
    bool flag = client.StreamData(processor);
    if (!flag) {
        std::cout << "Failed to stream data" << std::endl;
        return 1;
    }
    std::cout << "Stream for 10 seconds" << std::endl;
    std::chrono::steady_clock::time_point last = std::chrono::steady_clock::now();
    // stream for 10 seconds
    while (true) {
        auto now = std::chrono::steady_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - last);
        if (delta.count() > 10000) {
            break;
        }
    }
    std::cout << "Stopping stream" << std::endl;
    client.StopStreaming();
    return 0;
}
