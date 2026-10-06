//
// Created by Chris Davey on 22/9/2026.
//

#ifndef SDRCOMMS_CONTANTS_H
#define SDRCOMMS_CONTANTS_H

namespace sdrcomms {
    /**
     * the default Mega Samples Per Second for sampling.
     * Devices can support higher sample rates, but over USB 2.0 this
     * becomes challenging for continuous sampling.
     *
     *
     */
    const double DEFAULT_SAMPLE_RATE_MSPS = 3.0e6;

    /**
     * default queue depth.
     * this controls the size of the circular buffer used to store samples.
     * it should be large enough to allow the processing threads to keep up with the incoming data.
     */
    const int DEFAULT_QUEUE_SIZE = 128;

    /**
     *
     * default center frequency. 2.4Ghz
     */
    const double DEFAULT_CENTER_FREQUENCY = 2.4e7;

    /**
     * a step of 100 Hz
     */
    const double FREQUENCY_STEP = 1000;

    /**
     * step frequency every n millis.
     */
    const long long FREQUENCY_SCAN_CHANGE_MILLIS = 30 * 1000;

    /**
     * default minimum transmission unit
     * This is set in bytes.
     * 4096
     * The remote server will convert this into samples
     * because the remote server uses CS16, it will convert it into
     * MTU/4 samples in this case that is 1006 samples.
     * To use a sample preferred sample size power of 2, we should calculate
     * desired samples * 4
     * 1024*4 = 4096
     * this results in smaller sample sizes.
     */
    const size_t DEFAULT_MTU = 4096;
    /**
     * default window depth.
     * size of the kernel buffer in bytes.
     * this controls the size of the buffer in the remote server.
     */
    const size_t DEFAULT_WINDOW_SIZE = 4096;

    /**
     * read this many samples per block.
     */
    const size_t REQUIRED_SAMPLES = 65536;

    /**
     * stream timeout for udp clients
     * after which they will be removed.
     */
    const int UDP_CLIENT_STREAM_TIMEOUT = 60;

    /**
     * connected clients send a heartbeat every 5 seconds.
     */
    const int HEARTBEAT_MS = 5000;

    /**
     * default UDP port
     */
    const int DEFAULT_UDP_PORT = 45557;

    /**
     * practical limits of amount of UDP data to send
     * per chunk.
     */
    const int UDP_BYTES_PER_CHUNK = 1024;

}

#endif //SDRCOMMS_CONTANTS_H
