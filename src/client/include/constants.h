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
    const double DEFAULT_SCAN_MSPS_USB2 = 3.0e6;

    /**
     * default queue depth.
     * this controls the size of the circular buffer used to store samples.
     * it should be large enough to allow the processing threads to keep up with the incoming data.
     */
    const int DEFAULT_QUEUE_SIZE = 128;

    /**
     * default center frequency.
     */
    const double DEFAULT_CENTER_FREQUENCY = 100e6;
}

#endif //SDRCOMMS_CONTANTS_H
