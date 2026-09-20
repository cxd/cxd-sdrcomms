#include <iostream>
#include "client.h"
#include "SoapySDR/Modules.hpp"
#include <vector>
int main() {
    std::cout << "Initialising SoapySDR" << std::endl;
    SoapySDR::loadModules();
    std::vector<std::string> paths = SoapySDR::listSearchPaths();
    std::cout << "Module Search Paths" << std::endl;
    for (auto path : paths) {
        std::cout << path << std::endl;
    }
    std::cout << std::endl;

    std::vector<std::string> modules = SoapySDR::listModules();
    std::cout << "Available Modules" << std::endl;
    for (auto module : modules) {
        std::cout << module << std::endl;
    }

    sdrcomms::Client client {"tcp://192.168.0.102:11111", "remote"};
    bool isConnectd = client.Connect();
    std::cout << "Client connected: " << isConnectd << std::endl;
    return 0;
}
