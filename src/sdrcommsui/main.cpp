//
// Created by Chris Davey on 29/9/2026.
//
#include <iostream>
#include <CLI/CLI.hpp>
#include <csignal>
#include <thread>
#include <unistd.h>

#include "constants.h"
#include "ControlMessage.h"
#include "MessageQueue.h"
#include "MQTTServer.h"
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

    std::string mqttServerUri = "127.0.0.1";
    CLI::Option *mqttServerUriOpt = app.add_option("-m,--mqtt-server-address", mqttServerUri,
                                                   "The IP Address for the MQTT server eg: 127.0.0.1");
    int mqtt_port = sdrcomms::DEFAULT_MQTT_PORT;
    CLI::Option *mqttPortOpt = app.add_option("-t,--mqtt-server-port", mqtt_port,
                                              "The port for the MQTT server ie: 1833");


    CLI11_PARSE(app, argc, argv);
    std::cout << "Server Address: " << serverUri << std::endl;
    std::cout << "UDP Stream Port: " << udp_stream_port << std::endl;
    std::cout << "MQTT Address: " << mqttServerUri << std::endl;
    std::cout << "MQTT Port: " << mqtt_port << std::endl;

    // use the serverUri and udp_stream_port to setup and start the UDP client.
    // then setup the SDL view.
    if (serverUriOpt->count() == 0 || udpStreamPortOpt->count() == 0) {
        // print help
        std::cout << "Receive Spectrum Display UI " << std::endl;
        std::cout << app.help() << std::endl;
        return 1;
    }

    // the message queue used to send to clients
    auto sendToServerQueue = std::make_shared<sdrcomms::MessageQueue<sdrcomms::ControlMessage> >(
        sdrcomms::DEFAULT_QUEUE_SIZE);

    std::vector<std::string> subscriptions = {
        "sdrcomms/classify",
        "sdrcomms/ack/server",
        "sdrcomms/nack/server"
    };

    // we want to initialise the mqtt server to handle control channel communications.
    auto mqttServer = std::make_shared<sdrcomms::MQTTServer>(mqttServerUri, mqtt_port, sendToServerQueue,
                                                             "sdrcommsui::DisplayUI",
                                                             subscriptions);
    if (!mqttServer->init() || !mqttServer->start()) {
        std::cerr << "Could not connect to mqtt service: " << mqttServerUri << ":" << mqtt_port << std::endl;
        mqttServer->stop();
        return 1;
    }
    // instantiate the display.
    sdrcommsui::Display display(serverUri, udp_stream_port,
                                sdrcomms::REQUIRED_SAMPLES,
                                800,
                                600,
                                mqttServer->getMessageQueue(),
                                sendToServerQueue);

    // handle the close operations and shutdown the UDP client.
    struct sigaction sig_int_handler;
    sig_int_handler.sa_handler = signal_handler;
    sigemptyset(&sig_int_handler.sa_mask);
    sig_int_handler.sa_flags = 0;
    // register the handler for SIGINT (Ctrl+C)
    sigaction(SIGINT, &sig_int_handler, nullptr);
    sigaction(SIGTERM, &sig_int_handler, nullptr);

    std::cout << "Sleeping 2 seconds before starting" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    display.run();

    // stream until interrupted
    while (keep_alive) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    // cleanup
    display.stop();
    mqttServer->stop();
    return 0;
}
