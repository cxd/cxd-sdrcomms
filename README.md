# sdr-comms - a project for SDR integration for modulation classification tasks



This project provides a shell for experimenting with the integration of modulation classification models that have been exported in [ONNX](https://onnx.ai) format in a research setting.

**This project explicitly does not deal with the development of modulation classifier models themselves. Experiments for that purpose will be provided in a separate repository.**

This project looks at integration of such models within a wider system incorporating SDR, control, network and client layers. 

This project is licensed under the [Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License](https://creativecommons.org/licenses/by-nc-sa/4.0/) 

<img src="https://github.com/cxd/cxd-sdrcomms/raw/refs/heads/main/docs/assets/88x31.png?sanitize=true" alt="Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License" />


To cite this project use the following:

```
@misc{site_key,
  author       = {{Chris Davey}},
  title        = {sdr-comms - a project for SDR integration for modulation classification tasks},
  year         = {2026},
  howpublished = {\url{https://github.com/cxd/cxd-sdrcomms}},
  note         = {Accessed: YYYY-MM-dd}
}
```

where ```YYYY-MM-dd``` is the date of access.

## Why Modulation Classification?

Modulation Classification acts as part of an overall suite of tools in the application of "spectral awareness", which is a subset of "situational awareness" applications. 

In particular, as the use of autonomous vehicles such as drones, or wheeled platforms becomes increasingly common. It becomes important to detect their presence from a distance, this may be achieved through a combination of modulation classification and radio frequency fingerprinting. Similarly, activities in various fields rely on use of the spectrum for data transmission. This may be paired with geolocation, and deployed on vehicular platforms or fixed structures. Such an application when paired with visual sensing and other sensing apparatus can be used to generate a localised map of physical terrain. Spectrum use becomes an overlay in that map.

However, there are significant challenges (not addressed in this project), which provide opportunities for investigation such as:

- RF sensing from a moving platform suffers from distortions such as doppler effect, how do we compensate for this in the downstream tasks?
- Channel environments change with changing terrain, how would channel estimation help downstream tasks?
- The decision to actually run the classifier on incoming IQ Values must make a decision "is this noise? or might it contain a signal?". What are the best ways to make this decision in a blind signal setting?
- The available spectrum is a large space, meaning there is opportunity to explore search methods, and coordination of multiple collaborating actors scanning the environment.

Companion tasks also include RF fingerprinting, which aims to identify the nature of the device, or at least transmitter that is detected. This differs from modulation classification, but provides both data enrichment and potential for physical layer security in upstream tasks.

This is a rich area of potential research, with many related problems to explore.

## Why a Focus on Integration?

Many research articles focus primarily on the development of the model itself, and perhaps preprocessing. However integration of the model into a larger system is often overlooked. This is a non-trivial task, and one that provides an opportunity to investigate the practical challenges. 

This project provides a shell for the exploring the integration challenges.

Overall the integration has been an exercise working out how to combine all of the pieces that would comprise a blind modulation classification system as well as testing on multiple platforms including raspberry pi (if anyone wants to supply a jetson nano, this would also be a target of interest).

The following points are areas where the current integration has focused on:

- use of SDR device to capture IQ values and perform frequency scanning
- integration with preprocessing such as resampling IQ signal and FFT using hardware acceleration (vkFFT, and liquid-dsp libraries).
- integration with ONNX runtime to execute machine learning models
- network integration for remote control of the scanning operations (stop/start auto scanning, set frequency)
- network integration for streaming IQ values over UDP in custom streaming protocol
- network integration for messaging using MQTT for control and classification events
- management of multiple processing queues for receiving streams from SDR, processing, model classification and network communications,
- testing cross compilation against arm64 for raspberry pi targets.

## High Level Architecture

The server application is divided into four main areas of concern.

- SDR Integration
  - Is responsible for configuration of the SDR device. In this case we use the SoapySDR interface supporting remote or local devices.
  - Control component is also responsible for implementing the simple automatic scanning process. However, in a more advanced system the search algorithm could become a fifth area of concern. 
- Streaming
  - Streaming consumes the IQBlocks emitted by the SDR and supports sending to multiple client endpoints in a custom chunking protocol over UDP.
- Processing
  - The Processing component assembles a processing chain that may perform a series of transformations or alternately take various measurements from the incoming IQ data. The processing chain is also responsible for integration with the classification model.
- Messaging
  - Within the process data is exchanged over queues. 
  - Externally, the system leverages a separate Mosquitto server to enable publication to topics. MQTT was chosen as it is a common mechanism in IoT and has low overhead for integration.
  - Messaging over MQTT via tcp consumed by the client.

The application layer leverages the core library with some application specific components to assemble the base functionality of the server. This is defined in the ```application``` source directory. 
The common library is defined in the ```client``` source directory.

To display outputs from the server and interact with the basic configuration of the controller, a client UI is defined to display a IQ time/frequency waterfall of received IQ data and the most recent frequency spectrum. 
It also leverages an inset panel to display information for latest received timestamps, to allow the user to configure automatic scanning or manual scanning and to enter a center frequency in Hz. The client components are defined in the ```sdrcommsui``` source directory.

A conceptual overview of the system is described in the diagram below.

```mermaid
---
title: Overview
---
flowchart TB
    subgraph Server   
        subgraph SDR Integration
            Controller -. configures .-> SDRClient
            SDRClient -. connects .-> SDR
            SDRClient -. emits .-> IQBlocks
        end
        subgraph Streaming
            Application -. receives .-> IQBlocks
            Application -. streams via udp .-> IQBlockStream
        end
        subgraph Processing
            Application -. defines .-> ProcessingChain
            ProcessingChain -. processes .-> IQBlocks
            ProcessingChain -. performs .-> Preprocessing
            ProcessingChain -. loads .-> ClassifierModel
            ClassifierModel -. consumes .-> Preprocessing
            ClassifierModel -. emits .-> ClassificationEvents
        end
        subgraph Messaging
            MessageChannel -. emits .-> ClassificationEvents
            MessageChannel -. receives .-> ControlMessages
            ControlMessages -. configure .-> Controller 
        end
    end
    
    subgraph Client
        ClientUI -. connects to .-> MessageChannel
        ClientUI -. connects to .-> IQBlockStream
        ClientUI -. visualises .-> IQBlockStream
        ClientUI -. displays .-> ClassificationEvents
        ClientUI -. configures .-> ControlMessages
    end
    
```

## Example Client UI

This project includes an example client UI which receives IQ Value streams over UDP and the classification events over MQTT on the TCP port. It provides an example waterfall visualisation for the IQ Value stream, the current frequency graph (both in SDL), and a small control interface (in ImGui), to allow viewing current data rates, controlling whether to automatically scan or to manually set the center frequency.

An example is shown below.

https://github.com/cxd/cxd-sdrcomms/raw/refs/heads/main/docs/assets/screen_recording_001.mp4?sanitize=true


Typically the client may connect intermittently over a WiFi connection to the server (as in the case when the SDR and server are deployed on a mobile platform such as a drone). The wifi connection will be challenged to deliver the UDP stream at a large distance. As the stream can mean gigabytes of data transfer over a duration of time. However, classification events are small packets. When on board an autonomous platform, the IQ Value stream may not be used at all, instead classification events may be sent over MQTT locally to another controller process that is onboard the platform. The event paired with other sensor data may then be repackaged and issued to a connected control system downstream. 

# Dependencies

Requires installed libraries

```aiignore
sudo apt install libiio libiio-dev libad9361-0 libad9361-dev
```

Requires soapy remote

```aiignore
sudo apt install soapyremote-server soapysdr-tools
```

# Building on ARM

The dependencies for libiio need to be built on raspberry pi directly.

```aiignore
sudo apt install cmake git build-essential libusb-1.0-0-dev libxml2-dev bison flex libsoapysdr-dev

```

After compiling dependencies 
and installing the server it should be possible to confirm
the SoapySDRUtil can find it:

```aiignore
SoapySDRUtil --find "driver=plutosdr"
```

This should print out a list of interfaces.

```aiignore

Found device 0
  device = plutosdr
  driver = plutosdr
  uri = ip:pluto.local

Found device 1
  device = plutosdr
  driver = plutosdr
  uri = local:

Found device 2
  device = plutosdr
  driver = plutosdr
  uri = usb:3.16.5

```

```aiignore
SoapySDRUtil --probe="driver=plutosdr"
```

We configure the server to listen on port 1111

edit /

```aiignore
sudo vi /etc/systemd/system/SoapySDRServer.service 
```
to search the remote server and see if it has access to the devices
soapysrd util can be used.

```aiignore
 SoapySDRUtil --find="driver=remote,remote=tcp://192.168.0.102:11111"
```

Note the package soapyremote may need to be installed locally to allow access
by soapysdr util to the soapy remote server.


#### Contributing ####

I joke that in the age of AI, people will only write source code for the same reason they whittle wooden spoons. So aside from googling and searching for example use cases and yes, asking google gemini for advice on how to use unfamiliar APIs, this more often than not an example of a handmade code base. 

![Handmade](https://github.com/cxd/cxd-sdrcomms/raw/refs/heads/main/docs/assets/Handprint.png)

Handprint by _RootOfAllLight, CC BY-SA 4.0, via Wikimedia Commons_

For this project its fine to google and search for information about how to use apis, or get various hints on language usage, but for the most part, I'd prefer manual contributions. You can always fork the project and run wild with agents if you want to, in your own space.
