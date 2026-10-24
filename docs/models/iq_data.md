# IQ Data and implications for processing

The models are trained on different data sets.

These data sets may be generated with different sampling rates and different length samples.

For example:

## Panoradio
https://panoradio-sdr.de/radio-signal-classification-dataset/
- sample rate: 6kHz
- 2048 samples per sample

## Processing

Internally our sample rate may differ when sampling from the SDR as it is dependent on the SDR.

Model inputs may also differ in length and preprocessing steps.
We need a method of identifying the correct preprocessing steps for a given model.

## Model meta-data file:

Suggest to use a json file which can declare the input requirements and sample rate
used by the model.

```aiignore
{
"name": "Name of Model",
"version": "MODEL_VERSION_NUMBER_OR_TAG",
"onnx_file": "/path/to/model.onnx",
"sample_rate_Hz": 6000,
"input_length": 2048,
"input_transforms": ["iq", "fft"]
}
```

The json file provides metadata that can declare which types of transforms
need to be applied to the model file.
The sample rate declares what sample rate the model expects.
This means the processing pipeline will need to be aware of the downsampling required
for our current sample rate.
The sample rate cannot exceed the sample rate of the SDR.

By default we sample at 3 MHz (3000 kHz, 3000000 Hz).
Which means for example, the model expects a downsampled signal.

For 1024 samples at 3Mhz this is 0.34 ms.
For 1024 samples at 6Mhz this is 0.17 ms.

We need to downsample before feeding to the model.

For example we need to decimate the signal by 500 samples.
So are target sample rate is actually:

3000000 / 6000 Hz = 500 Hz

We need an antialiasing low pass filter on the SDR stream as one of our processing steps.


