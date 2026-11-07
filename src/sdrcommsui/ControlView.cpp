//
// Created by Chris Davey on 7/11/2026.
//

#include "include/ControlView.h"

#include <imgui.h>

namespace sdrcommsui {

    ControlView::ControlView(std::string udpServer, int port, double centerFreq,
            std::shared_ptr<sdrcomms::MessageQueue<sdrcomms::ControlMessage>> inboundMessageQueue,
            std::shared_ptr<sdrcomms::MessageQueue<sdrcomms::ControlMessage>> outboundMessageQueue) {
        this->udpServer = udpServer;
        this->port = port;
        this->centerFreq = centerFreq;
        this->inboundMessageQueue = inboundMessageQueue;
        this->outboundMessageQueue = outboundMessageQueue;
    }


    /**
        * Draw the spectrum waterfall.
        * @param renderer
        * @param width
        * @param height
        * @param timestep
        * @param frame
        * @param fps
        */
    void ControlView::DrawSketch(SDL_Renderer *renderer, int width, int height, int timestep, int frame, int fps) {
        // TODO: draw ImGui Controls.

        ImGui::Text("Server: %s", udpServer.c_str());
        ImGui::Text("Port: %d", port);
        std::string centerFreqStr = std::to_string(centerFreq);
        //ImGui::InputText("Center Frequency Hz", centerFreqStr.c_str());
        ImGui::Text("Center Frequency: %f", centerFreq);
        bool scanFrequency = autoScan;
        bool newScanFlag = ImGui::Checkbox("Scan Frequency Automatically", &scanFrequency);

        // check if the autoScan setting has changed from true to false.
        if (!newScanFlag && autoScan) {
            // TODO: send message to stop automatic scanning.
            //
        }
    }

}
