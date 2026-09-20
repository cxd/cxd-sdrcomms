# SDR Comms



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

