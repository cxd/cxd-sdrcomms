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
"sample_rate_hz": 6000,
"input_length": 2048,
"input_transforms": ["downsample", "fft"]
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

Since we have vulkan enabled FFT we might be able to perform FFT based resampling - frequency domain decimation.

So we perform an FFT at high resolution.
Then use FFT shift to move 0 Hz into the center.
We then take a plus minus 3Khz around the center of the spectrum.
Then we use the IFFT to convert back to the time domain.

We also need to rescale the amplitude of the resulting time domain.

One downside is the Gibbs phenomenon.

Alternate approach is a Polyphase FIR Decimation.

The number of required samples to capture is based on the ratio of the
original sampling rate in Hz (the target Hz we need to downsample into) 
and the device sampling rate in Hz.

If the target samples size is 2048 and the target sample rate was 6e3 Hz and we sample at 3e6 Hz
Then:

```
required_samples_size = target_samples_size / (target_rate_hz / source_rate_hz))
```

eg:

$r = (6e3/3e6) = 0.002$

$2048 / r = 1024000 samples$

But if a model is trained at a given sampling rate,
we are assuming the transmission sampling rate is the same as the model sample rate.

However the challenge is in detecting modulations, that
the transmission sampling rates are variable.

Sampling rate is scale, this is similar to a multi-resolution scaling problem.





# Time to samples:

$$
N = F_s \times T
$$

where $N$ is the number of samples, $F_s$ is the sample rate in Hz and $T$ is the capture duration in seconds. 
This gives samples per duration in seconds.

FFT Resolution

$$
\Delta f = \frac{F_s}{N} = \frac{1}{T}
$$

$N$ is the number of samples.

When we think about the duration of time required to capture a sample
we need to look at the size of the frame in the message.
For example if we have a Minimum transmission unit of 4098 samples at a sample rate of 3e6 Hz

We can think about the duration of time required to capture a single frame when using a 3e6 Hz sample rate.

$$
T = \frac{4096}{3e6} \approx 0.001365 s = 1.365 ms
$$

But that is only if the transmission sample rate is 3e6 Hz as well.





