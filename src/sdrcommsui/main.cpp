//
// Created by Chris Davey on 29/9/2026.
//
#include <iostream>
#include <CLI/CLI.hpp>
int main(int argc, char **argv) {
    std::cout << "Initialising SoapySDR" << std::endl;
    CLI::App app;
    std::string serverUri = "127.0.0.1";
    CLI::Option *serverUriOpt = app.add_option("-s,--server-address", serverUri,
                                               "The IP Address for the UDP application server eg: 127.0.0.1");
    int udp_stream_port = 15555;
    CLI::Option *udpStreamPortOpt = app.add_option("-p,--udp-stream-port", udp_stream_port,
                                                    "The port for the udp stream ie: 15555");


    CLI11_PARSE(app, argc, argv);
    std::cout << "Server Address: " << serverUri << std::endl;
    std::cout << "UDP Stream Port: " << udp_stream_port << std::endl;

    // use the serverUri and udp_stream_port to setup and start the UDP client.
    // then setup the SDL view.

    // handle the close operations and shutdown the UDP client.
}