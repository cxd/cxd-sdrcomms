# Plutos SDR Configuration

Setting configuration for plutosdr

https://wiki.gnuradio.org/index.php/PlutoSDR_FMRadio

https://wiki.analog.com/university/tools/pluto

https://developer.analog.com/docs/system-level/solutions/reference-designs/fmcomms2/index.html

https://github.com/analogdevicesinc/libiio/blob/main/examples/ad9361-iiostream.c

https://pysdr.org/content/pluto

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


