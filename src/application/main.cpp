#include <CLI/CLI.hpp>
#include <iostream>
#include "client.h"
#include "constants.h"
#include <memory>
#include <vector>
#include <chrono>
#include <atomic>
#include <csignal>
#include <unistd.h>
#include "ApplicationProcessor.h"
#include "MockProcessor.h"


volatile sig_atomic_t keep_alive = 1;

/**
 * we will install the signal hanlder to capture the interrupt.
 * @param signal
 */
void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        keep_alive = 0;
    }
}

/*
 * using argument parser CLI11
 * https://cliutils.github.io/CLI11/book-flags.html
 */

int main(int argc, char **argv) {
    std::cout << "Initialising SoapySDR" << std::endl;
    CLI::App app;
    std::string serverUri;
    CLI::Option *serverUriOpt = app.add_option("-s,--server-uri", serverUri,
                                               "The Uri for the soapyremote-server eg: tcp://ipaddr:port");
    std::string driverName;
    CLI::Option *driverOpt = app.add_option("-d,--driver-name", driverName, "The name of the local driver ie: remote");
    std::string remoteDriver;
    CLI::Option *remoteDriverOpt = app.add_option("-r,--remote-driver", remoteDriver,
                                                  "The name of the remote driver ie: plutosdr");
    std::string deviceUri;
    CLI::Option *deviceOpt = app.add_option("-u,--device-uri", deviceUri, "The uri for the device ie: ip:pluto.local");
    std::string bindAddress;
    CLI::Option *bindOpt = app.add_option("-b,--bind-address", bindAddress,
                                          "The bind address for the remote server ie: 192.168.0.102");

    std::string udpServerUri = "0.0.0.0";
    CLI::Option *udpServerUriOpt = app.add_option("-a,--udp-server-address", udpServerUri,
                                               "The IP Address for the UDP application server eg: 127.0.0.1");

    int udp_stream_port = sdrcomms::DEFAULT_UDP_PORT;
    CLI::Option *udpStreamPortOpt = app.add_option("-p,--udp-stream-port", udp_stream_port,
                                                    "The port for the udp stream ie: 15555");


    CLI11_PARSE(app, argc, argv);
    // these params can be used to connect to local device directly
    // if specified.
    if (driverOpt->count() == 0 || deviceOpt->count() == 0) {
        std::cout << "Soapy Receiver Client" << std::endl;
        std::cout << app.help() << std::endl;
        return 1;
    }
    // these params are optionally used for remote connections.
    if (serverUriOpt->count() == 0) {
        serverUri = "";
    }
    if (bindOpt->count() == 0) {
        bindAddress = "";
    }
    if (remoteDriverOpt->count() == 0) {
        remoteDriver = "";
    }
    if (udpStreamPortOpt->count() == 0) {
        udp_stream_port = sdrcomms::DEFAULT_UDP_PORT;
    }
    /* Example
    --server-uri="tcp://192.168.0.102:11111"
        --driver-name="remote"
        --remove-driver="plutosdr"
        --device-uri="ip:pluto.local"
        --bind-address="192.168.0.102"
    */
    sdrcomms::Client client{
        serverUri,
        driverName,
        remoteDriver,
        deviceUri,
        bindAddress,
        sdrcomms::DEFAULT_QUEUE_SIZE
    };
    bool isConnectd = client.Connect();
    std::cout << "Client connected: " << isConnectd << std::endl;
    if (isConnectd) {
        client.PrintSoapySDRInfo();
    } else {
        std::cerr << "Could not connect to soapysdr server" << std::endl;
        return 1;
    }
    client.SetSampleRateMSps(sdrcomms::DEFAULT_SAMPLE_RATE_MSPS);
    client.SetCenterFrequency(sdrcomms::DEFAULT_CENTER_FREQUENCY);
    auto processor = std::make_shared<ApplicationProcessor>(udpServerUri, udp_stream_port);
    // start the processor server to allow remote connections.
    processor->startServer();

    bool flag = client.StreamData(processor);
    if (!flag) {
        std::cout << "Failed to stream data" << std::endl;
        return 1;
    }
    std::cout << "Begin Stream use Ctrl+C to exit" << std::endl;

    struct sigaction sig_int_handler;
    sig_int_handler.sa_handler = signal_handler;
    sigemptyset(&sig_int_handler.sa_mask);
    sig_int_handler.sa_flags = 0;
    // register the handler for SIGINT (Ctrl+C)
    sigaction(SIGINT, &sig_int_handler, nullptr);
    sigaction(SIGTERM, &sig_int_handler, nullptr);

    // stream until interrupted
    while (keep_alive) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "Stopping stream" << std::endl;
    client.StopStreaming();
    processor->stopServer();
    return 0;
}
