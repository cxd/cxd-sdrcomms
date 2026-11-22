# Raspberry Pi

Limitations appear to be around wifi.

The raspberry pi performs well in reading from the SDR, preprocessing IQ blocks and classification.

It is the speed of the wifi network link to broadcast IQ samples over the link to UDP clients that is slow. This is of course a challenge.

Check signal strength:

```
sudo iwconfig wlan0
```

The wifi channel can be checked:

```
sudo nmcli -f IN-USE,SSID,BSSID,CHAN,FREQ,SIGNAL dev wifi list
```

And the connection identified with:

```
sudo nmcli connection show
```

We may need to select the 5GHz channel if it is available.

```
nmcli connection modify "<connection-name>" 802-11-wireless.band a
nmcli connection modify "<connection-name>" 802-11-wireless.channel 36
nmcli connection up "<connection-name>"
```

Depends on the channel you use for 5GHz.


The raspberry pi is a low power device, so distance from wireless transmitter for streaming UDP will have a big impact on network streaming performance.

However it could be connected over ethernet to provide more reliable network for UDP streaming.