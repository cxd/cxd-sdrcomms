# Some challenges in modelling.

have built several models for a simple data set available at https://panoradio-sdr.de/radio-signal-classification-dataset/
This is just to get some idea of how integration with a model might work by loading in an ONNX format on the device.

Some interesting challenges that would present in the future:

- data generation - obtaining enough examples of modulation types.
- recording drone transmissions - this would be a interesting challenge in itself,  if there’s one available for purchase it might be a worthwhile investment, I haven’t seen any drone transmission data sets in the public domain yet.
- detection - determining when a signal is present to send to the classifier or ignore (does the classifier include examples of noise samples to train on?)
- resolution kHz - the example data sets have different resolutions in terms of sampleRate, does the model learn multiple sample rates or one sample rate?
- high sample rates would mean larger model parameters and slower training due to amount of data, larger models will be challenging for embedded devices to run.
- alternate approaches could specify how the sample rate needs to be resampled after streamed from the SDR before sending into the model.
- other types of preprocessing , FFT, STFT, etc.


There are a number of data sets including the one from the cyclostationary blog, and there’s a library available to assist in generating more called torchsig.
Accuracy of generated sample types may be challenging from what I’ve read, so having a library to do that would be interesting.

Eitherway, the researcher could likely spend alot of time working out how to do the modelling, without needing to dive into the integration challenges with the SDR.


# Timescales

Many of the datasets have different timescales associated with the generated source data.

This source data is assumed to be transmitted at varying sample rates.

The challenge is that this alters the timescale at which the model needs to operate, 
different modulations will have different transmit sample rates, and varying samples per symbol .

So there is a challenge in how the model needs to handle various changes of scale.

Preprocessing methods will need to be explored.

Some very interesting work on the cyclonstationary blog has provided a number of papers
discussing feature extraction methods using power spectrum and cyclostationary features as well as some success
with feeding these into capsule modules.

The major challenge also is having a broad range of modulations to train on that are accurate enough
to reliably be used for the model.

Scale of input sample rate is also a major challenge for the modelling task.

Multi-resolution models may be an area of investigation - this is an area
for example of a scaling scenario.
Do we modify the training data to use a standard scale for example, upsampling inputs from too low resolution, 
or downsampling training data for higher sample rates?



# Implementation challenges

ONNX format is the defacto interchange format to allow use of various training frameworks,
but support for ONNX export is not uniform between all frameworks.
Tensorflow has good support, but the import itself needs to be tested before it can be reliably deployed.


