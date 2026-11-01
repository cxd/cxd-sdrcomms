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
#include "Controller.h"
#include "MockProcessor.h"
#include "MQTTServer.h"


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

    std::string mqttServerUri = "127.0.0.1";
    CLI::Option *mqttServerUriOpt = app.add_option("-m,--mqtt-server-address", mqttServerUri,
                                               "The IP Address for the MQTT server eg: 127.0.0.1");
    int mqtt_port = sdrcomms::DEFAULT_MQTT_PORT;
    CLI::Option *mqttPortOpt = app.add_option("-t,--mqtt-server-port", mqtt_port,
                                                        "The port for the MQTT server ie: 1833");

    std::string modelPath = "";
    CLI::Option *modelPathOpt = app.add_option("-o,--model-path", modelPath,
                                                "The base path to the model files");
    std::string modelSpecFile = "";
    CLI::Option *modeSpecFileOpt = app.add_option("-f,--model-file", modelSpecFile,
        "The json file containing the model specification to use.");

    std::string classMapFile = "";
    CLI::Option *classMapFileOpt = app.add_option("-c,--class-map-file", classMapFile,
        "The json file containing the class map to use.");

    CLI11_PARSE(app, argc, argv);
    // these params can be used to connect to local device directly
    // if specified.
    if (driverOpt->count() == 0 || deviceOpt->count() == 0) {
        std::cout << "Soapy Receiver Client" << std::endl;
        std::cout << app.help() << std::endl;
        return 1;
    }

    if (modelPathOpt->count() == 0 || modelPath == "") {
        std::cout << "Model Path not specified." << std::endl;
        std::cout << app.help() << std::endl;
        return 1;
    }

    if (modeSpecFileOpt->count() == 0 || modelSpecFile == "") {
        std::cout << "Model Spec file not defined." << std::endl;
        std::cout << app.help() << std::endl;
        return 1;
    }

    if (classMapFileOpt->count() == 0 || classMapFile == "") {
        std::cout << "Class Map file not defined." << std::endl;
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
    auto client = std::make_shared<sdrcomms::Client>(
        serverUri,
        driverName,
        remoteDriver,
        deviceUri,
        bindAddress,
        sdrcomms::DEFAULT_QUEUE_SIZE,
        sdrcomms::REQUIRED_SAMPLES
    );
    bool isConnectd = client->Connect();
    std::cout << "Client connected: " << isConnectd << std::endl;
    if (isConnectd) {
        client->PrintSoapySDRInfo();
    } else {
        std::cerr << "Could not connect to soapysdr server" << std::endl;
        return 1;
    }
    client->SetSampleRateMSps(sdrcomms::DEFAULT_SAMPLE_RATE_HZ);
    client->SetCenterFrequency(sdrcomms::DEFAULT_CENTER_FREQUENCY);


    // the message queue used to send to clients
    std::shared_ptr<sdrcomms::MessageQueue<sdrcomms::ControlMessage>> sendToClientQueue = std::make_shared<sdrcomms::MessageQueue<sdrcomms::ControlMessage>> (sdrcomms::DEFAULT_QUEUE_SIZE);

    auto processor = std::make_shared<ApplicationProcessor>(udpServerUri,
        udp_stream_port,
        sdrcomms::DEFAULT_SAMPLE_RATE_HZ,
        sdrcomms::REQUIRED_SAMPLES,
        modelSpecFile,
        classMapFile,
        modelPath,
        sendToClientQueue);

    // initialise the application processor.
    if (!processor->init()) {
        client->StopStreaming();
        return 1;
    }
    // start the processor server to allow remote connections.
    processor->startServer();

    bool flag = client->StreamData(processor);
    if (!flag) {
        std::cout << "Failed to stream data" << std::endl;
        processor->stopServer();
        client->StopStreaming();
        return 1;
    }
    // we want to initialise the mqtt server to handle control channel communications.
    std::shared_ptr<sdrcomms::MQTTServer> mqttServer = std::make_shared<sdrcomms::MQTTServer>(mqttServerUri, mqtt_port, sendToClientQueue);
    if (!mqttServer->init() || !mqttServer->start()) {
        std::cerr << "Could not connect to mqtt service: " << mqttServerUri << ":" << mqtt_port << std::endl;
        mqttServer->stop();
        processor->stopServer();
        client->StopStreaming();
        return 1;
    }
    Controller controller {client, mqttServer->getMessageQueue()};
    controller.start();


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
    controller.stop();
    mqttServer->stop();
    processor->stopServer();
    client->StopStreaming();

    return 0;
}
