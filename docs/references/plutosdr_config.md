# Plutos SDR Configuration

Setting configuration for plutosdr

https://wiki.gnuradio.org/index.php/PlutoSDR_FMRadio

https://wiki.analog.com/university/tools/pluto

https://developer.analog.com/docs/system-level/solutions/reference-designs/fmcomms2/index.html

https://github.com/analogdevicesinc/libiio/blob/main/examples/ad9361-iiostream.c

https://pysdr.org/content/pluto

## Check addresses of connected devices.

```aiignore
iio_info -s
```

eg:

```aiignore
Available contexts:
	0: 192.168.2.1 (Analog Devices PlutoSDR Rev.C (Z7010-AD9363A)), serial=10447376de0b00090d0008009ada9723d5 [ip:pluto.local]
	1: (dell_smm,coretemp on Dell Inc.) [local:]
	2: 0456:b673 (Analog Devices Inc. PlutoSDR (ADALM-PLUTO)), serial=10447376de0b00090d0008009ada9723d5 [usb:3.4.5]
```

## SoapySDRUtil

```aiignore
SoapySDRUtil --find="driver=remote,remote=tcp://localhost:11111"
```

```aiignore
Found device 0
  device = PlutoSDR
  driver = remote
  label = PlutoSDR #0 usb:3.4.5
  remote = tcp://localhost:11111
  remote:driver = plutosdr
  uri = usb:3.4.5

Found device 1
  device = PlutoSDR
  driver = remote
  label = PlutoSDR #0 ip:pluto.local
  remote = tcp://localhost:11111
  remote:driver = plutosdr
  uri = ip:pluto.local
```

## Probe the device through the server:

```aiignore
SoapySDRUtil --probe="driver=remote,remote=tcp://localhost:11111,remote:uri=usb:3.4.5"
```

outputs

```aiignore
[INFO] Opening PlutoSDR #0 usb:3.4.5...

----------------------------------------------------
-- Device identification
----------------------------------------------------
  driver=PlutoSDR
  hardware=ADALM-PLUTO
  ad9361-phy,model=ad9363a
  ad9361-phy,xo_correction=40000005
  backend_version=0.24 (git tag: v0.24)
  fw_version=v0.37
  hw_model=Analog Devices PlutoSDR Rev.C (Z7010-AD9363A)
  hw_model_variant=1
  hw_serial=10447376de0b00090d0008009ada9723d5
  library_version=0.25 (git tag: v0.25)
  local,kernel=5.10.0-98725-g3eae70065be9
  remote:version=0.6.0-gd09903c8
  uri=usb:3.4.5
  usb,idProduct=b673
  usb,idVendor=0456
  usb,libusb=1.0.27.11882
  usb,product=PlutoSDR (ADALM-PLUTO)
  usb,release=2.0
  usb,serial=10447376de0b00090d0008009ada9723d5
  usb,vendor=Analog Devices Inc.

----------------------------------------------------
-- Peripheral summary
----------------------------------------------------
  Channels: 1 Rx, 1 Tx
  Timestamps: NO
  Sensors: xadc_temp0, xadc_voltage0, xadc_voltage1, xadc_voltage2, xadc_voltage3, xadc_voltage4, xadc_voltage5, xadc_voltage6, xadc_voltage7, xadc_voltage8, adm1177_current0, adm1177_voltage0, ad9361-phy_temp0, ad9361-phy_voltage2
     * xadc_temp0: 52.784491 C
     * xadc_voltage0 (vccint): 1.014404 V
     * xadc_voltage1 (vccaux): 1.787842 V
     * xadc_voltage2 (vccbram): 1.010742 V
     * xadc_voltage3 (vccpint): 1.005615 V
     * xadc_voltage4 (vccpaux): 1.784912 V
     * xadc_voltage5 (vccoddr): 1.343262 V
     * xadc_voltage6 (vrefp): 1.248047 V
     * xadc_voltage7 (vrefn): -0.000732 V
     * xadc_voltage8: 0.801514 V
     * adm1177_current0: 
     * adm1177_voltage0: 
     * ad9361-phy_temp0: 35.088000 C
     * ad9361-phy_voltage2: 0.551892 V

----------------------------------------------------
-- RX Channel 0
----------------------------------------------------
  Full-duplex: YES
  Supports AGC: YES
  Stream formats: CS8, CS12, CS16, CF32
  Native format: CS16 [full-scale=2048]
  Stream args:
     * Remote Format - The stream format used on the remote device.
       [key=remote:format, default=CS16, type=string, options=(CS8, CS12, CS16, CF32)]
     * Remote Scale - The factor used to scale remote samples to full-scale floats.
       [key=remote:scale, default=2048.000000, type=float]
     * Remote MTU - The maximum datagram transfer size in bytes.
       [key=remote:mtu, units=bytes, default=1500, type=int]
     * Remote Window - The size of the kernel socket buffer in bytes.
       [key=remote:window, units=bytes, default=44040192, type=int]
     * Remote Priority - Specify the scheduling priority of the server forwarding threads.
       [key=remote:priority, default=0.500000, type=float, range=[-1, 1]]
     * Remote Protocol - Specify the transport protocol for the remote stream.
       [key=remote:prot, default=udp, type=string, options=(udp, tcp, none)]
  Antennas: A_BALANCED
  Full gain range: [0, 73] dB
    PGA gain range: [0, 73] dB
  Full freq range: [70, 6000] MHz
    RF freq range: [70, 6000] MHz
  Sample rates: [0.0651042, 61.44] MSps
  Filter bandwidths: 0.2, 1, 2, 3, 4, 6, 7, 8, 9, 10 MHz

----------------------------------------------------
-- TX Channel 0
----------------------------------------------------
  Full-duplex: YES
  Supports AGC: NO
  Stream formats: CS8, CS12, CS16, CF32
  Native format: CS16 [full-scale=32768]
  Stream args:
     * Remote Format - The stream format used on the remote device.
       [key=remote:format, default=CS16, type=string, options=(CS8, CS12, CS16, CF32)]
     * Remote Scale - The factor used to scale remote samples to full-scale floats.
       [key=remote:scale, default=32768.000000, type=float]
     * Remote MTU - The maximum datagram transfer size in bytes.
       [key=remote:mtu, units=bytes, default=1500, type=int]
     * Remote Window - The size of the kernel socket buffer in bytes.
       [key=remote:window, units=bytes, default=44040192, type=int]
     * Remote Priority - Specify the scheduling priority of the server forwarding threads.
       [key=remote:priority, default=0.500000, type=float, range=[-1, 1]]
     * Remote Protocol - Specify the transport protocol for the remote stream.
       [key=remote:prot, default=udp, type=string, options=(udp, tcp, none)]
  Antennas: A
  Full gain range: [0, 89] dB
    PGA gain range: [0, 89] dB
  Full freq range: [70, 6000] MHz
    RF freq range: [70, 6000] MHz
  Sample rates: [0.0651042, 61.44] MSps
  Filter bandwidths: 0.2, 1, 2, 3, 4, 6, 7, 8, 9, 10 MHz
```

## Recommended sampling rates:

70 MHz to 6 GHz

because it only has USB 2.0, the 56 MHz can only be received in short bursts.

continuously receiving is more like 4 or 5 MHz

3MHz (3000000 Hz) eg:

```aiignore
double sampleRate = 3.0e6;
device->setSampleRate(SOAPY_SDR_RX, chan, sampleRate);
device->setBandwidth(SOAPY_SDR_RX, chan, sampleRate);
```

## Bandwidth limits:

limits: >= 200000 and <= 52000000

## Rx Gain

Manual Gain (Rx1)(dB)
gain value, initially use 64




