# Analog Devices libiio and iio_utils

iio_utils can check the underlying connection to plutosdr.

On the mac the custom build is located in:

```aiignore
/Library/Frameworks/iio.framework/Tools/iio_info
```

On linux it is installed using iio_utils.

It is possible to scan for devices using:

```aiignore
sudo /Library/Frameworks/iio.framework/Tools/iio_info -s
```

or 

```aiignore
iio_info -s
```

To print out the details of the SDR device use:

```aiignore
sudo /Library/Frameworks/iio.framework/Tools/iio_info -u "usb:1.4.5"
```

or

```aiignore
iio_info -u "usb:1.4.5"
```


```aiignore
 SoapySDRUtil --find="remote=tcp://192.168.0.89:11111,remote:driver=plutosdr"
```