#include <iostream>
#include "client.h"

#include <vector>
int main() {
    std::cout << "Initialising SoapySDR" << std::endl;
    sdrcomms::Client client {"tcp://192.168.0.102:11111", "remote"};
    client.PrintSoapySDRInfo();
    bool isConnectd = client.Connect();
    std::cout << "Client connected: " << isConnectd << std::endl;
    if (isConnectd) {
        client.PrintDeviceInfo();
    }
    return 0;
}
