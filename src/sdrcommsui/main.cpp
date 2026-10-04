//
// Created by Chris Davey on 29/9/2026.
//
#include <iostream>
#include <CLI/CLI.hpp>
#include <csignal>
#include <thread>
#include <unistd.h>

#include "constants.h"
#include "include/display.h"
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


int main(int argc, char **argv) {
    std::cout << "Initialising SoapySDR" << std::endl;
    CLI::App app;
    std::string serverUri = "127.0.0.1";
    CLI::Option *serverUriOpt = app.add_option("-s,--server-address", serverUri,
                                               "The IP Address for the UDP application server eg: 127.0.0.1");
    int udp_stream_port = sdrcomms::DEFAULT_UDP_PORT;
    CLI::Option *udpStreamPortOpt = app.add_option("-p,--udp-stream-port", udp_stream_port,
                                                    "The port for the udp stream ie: 15555");


    CLI11_PARSE(app, argc, argv);
    std::cout << "Server Address: " << serverUri << std::endl;
    std::cout << "UDP Stream Port: " << udp_stream_port << std::endl;

    // use the serverUri and udp_stream_port to setup and start the UDP client.
    // then setup the SDL view.
    if (serverUriOpt->count() == 0 || udpStreamPortOpt->count() == 0) {
        // print help
        std::cout << "Receive Spectrum Display UI " << std::endl;
        std::cout << app.help() << std::endl;
        return 1;
    }
    // instantiate the display.
    sdrcommsui::Display display(serverUri, udp_stream_port, sdrcomms::REQUIRED_SAMPLES);

    // handle the close operations and shutdown the UDP client.
    struct sigaction sig_int_handler;
    sig_int_handler.sa_handler = signal_handler;
    sigemptyset(&sig_int_handler.sa_mask);
    sig_int_handler.sa_flags = 0;
    // register the handler for SIGINT (Ctrl+C)
    sigaction(SIGINT, &sig_int_handler, nullptr);
    sigaction(SIGTERM, &sig_int_handler, nullptr);

    display.run();

    // stream until interrupted
    while (keep_alive) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    // cleanup
    display.stop();
    return 0;
}