# Detection Methods

The basic decision of whether or not the IQ samples from the SDR might contain a signal of interest.

The system needs to make the decision in a blind operation mode and is challenged by:

- some signals may be non-repeating
- signals are weak
- there may be multiple signals sharing the same frequency and bandwidth (narrow band and wideband).


Some ideas.

## 1. Adaptive Power Ratio (Short Term/Long Term Average)

Using the STA/LTA - Schmidl-Cox energy ratio we can look at the changes in gain and noise.

Compute two sliding moving averages of instantaneous signal power over two windows short $W_{s}$ and long $W_l$.
A short window may consist of 16-64 samples, a long window may consist of 512 - 2048 samples.
$$
W = \frac{1}{N} \sum_{i=0}^{N} |x_i|^2
$$
Where $N$ is the number of samples in each window.

The ratio is computed as $\frac{W_s}{W_l}$ or $\text{STA/LTA}$.

A threshold value is applied to determine whether a signal may be present.

It is fast and simple to implement, but may not be suitable for all scenarios.
Strong narrowband interference in the passband can trigger false positives.
Might miss low-power signals drowned out by wideband noise.

## 2. Constant False Alarm Rate (CFAR) in Frequency Domain

Rolling FFTs of a small overlapping window eg: 256 to 1024 points.
Compute Cell-Averaging CFAR (CA-CFAR) or Ordered Statistic CFAR (OS-CFAR) over frequency binds.

Use a threshold above a normal noise floor to determine whether the signal might be present.

## 3. Kurtosis

Test the kurtosis of the signal in comparison to the kurtosis of an assumed AGWN noise.

If the kurtosis varies from the noise it may contain a signal.

## 4. Autocorrelation

Take the autocorrelation of sliding windows over the signal.
Noise should have low autocorrelation, structured signals should produce high autocorrelation.

Disadvantage is that it fails to detect uncorrelated signals.
