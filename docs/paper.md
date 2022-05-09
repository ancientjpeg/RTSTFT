# THE RTSTFT FRAMEWORK

## Abstract
- What is it?
  - Pure C Library
  - A core STFT algorithm with several methods for manipulation
    - Per-bin manipulations, i.e. gating, limiting, muting
    - Phase vocoder based pitch-shifting
    - Freeform phase manipulation for effects akin to phase chorus
    - More to come...
  - A scripting language, rt_cmd, for precise control of these parameters
    - Includes a custom command parser
- What can it do?
  - User level manipulation of FFT bins, both amplitude and phase
    - Sonically akin to filtration, but with audible artifacts that can be creatively exploited
  - Methods for adjusting the phase vocoder algorithm for creative results
    - phase chorus
    - "robotization", but with more control
  - Intended for users to get in deep and experiment with + corrupt their sounds

## Background & History
#### Motivation and philosophy behind creation
- Digital audio is stuck trying to emulate and outperform analog gear
- Very little emphasis on exploring the creative possibilities that are unique to digital sound processing algorithms
- Few spectral plugins that aren't extremely nebulous about their actual internal function
- Very few creative plugins in general that emphasize surgical precision
- Absolutely no spectral plugins that allow direct interface with their underlying algorithms (to my knowledge)
  - RTSTFT is designed for user-level manipulation of individual FFT bins
- Also intended as a lightweight and friendly introduction to command-line syntax
  - A skill I think everyone deserves to be taught but lacks easily available resources for beginners
#### History of spectral audio
- Fourier transform began as a method for thermodynamic analysis
- Equalizers began as audio applications of principles used in electrical engineering
  - By their nature, smooth and somewhat imprecise (which is generally pleasing to the ear)
- Vocoders may be the first discretized audio processor
  - Utilized filters to isolate frequency bands for individual processing
- With the advent of modern computers, the DFT could be performed real-time
  - H910 Harmonizer
  - AutoTune (started out as oil location)
  - Phase vocoding for cell phones
  - JPEGs! (DCT)



## Mathematics

For this section explaining the math behind RTSTFT, I will be using *extremely* explicit notation. For any readers with a background in maths, it will likely seem like overkill, but I do so in order to create an explanation that I wish had existed when I was learning all of this on my own. The following explanation is intended for a anyone with only a fuzzy recollection of highschool-level calculus, and insists upon instilling the reader with a deep understanding of the actual mechanisms and reasoning behind the math instead of just the computations alone.

#### An Overview of the Fourier Transform

For some set of $N$ input values $x_{input}$, the Discrete Fourier Transform (DFT) will return an array of values $x_{output}$ at an index $k$ defined as:

$$\displaystyle x_{output}[k] = \sum_{n=0}^{N - 1}{x[n] \cdot e^{\frac{i2\pi kn}{N}}}$$ 


Note that this set of input values, or "signal" as it will be hereon referred to, is zero-indexed, hence taking the sum from 0 to N-1. This equation may certainly seem a little daunting, but we can break it down a bit by noting that it shares the form of Euler's formula, $e^{ix} = \cos{x} + i\sin{x}$. Applying this, we get\footnote{wikipedia}:

$$\displaystyle x_{output}[k] = \sum_{n=0}^{N - 1}{x[n] \cdot (\cos(\frac{i2\pi kn}{N}) + i\sin(\frac{i2\pi kn}{N}))}$$

Though this seems no less complicated, it tells us something very fundamental about the equation: it is returning a value that lies on some circle where the x-axis is real and the y-axis is imaginary. The DFT imagines the input signal as some addition of many different harmonic sine waves, where the lowest frequency sinusoid has a frequency of 1/N samples, and the highest a frequency of 2 samples. The inner portion of the equation, $x[n] \cdot \frac{i2\pi kn}{N}$, can be interpreted as the complex "responase" of a given frequency $\frac{1}{k}$ at a given sample $n$, with the sum of all these complex numbers representing the response

What matters is, when we take the absolute value and the angle of a complex number at some index $k$ of $x_{output}$ (corresponding to taking the radius and the angle of the complex number on its imaginary circle), the values we get back are the amplitude and the phase of the sinusoid with the frequency $\frac{1}{k}$ samples. 

Let me state that again in simpler terms for emphasis: the DFT decomposes any input signal into a set of sinusoid waves, and gives us the phase and amplitude of each of those sinusoids. This mathematical operation is not an estimation: it is 100% accurate and completely reversible. Even for people familiar with the DFT, the significance of this cannot be overstated: being able to decompose a signal like this, turning its time-domain information into frequency-domain information, is an invaluable tool in almost any field of mathematics that deals with periodic signals. This explanation still might not suffice for many, as even I still have trouble fully linking the variables in the equation to what's actually going on under the hood. I insist you go to 3blue1brown's YouTube Channel\footnote{footnote} and take a look at his video on the DFT; I find the visualizations to be extremely helpful in building a deeper understanding of how the DFT really works.

The DFT lies at the core of countless more complex signal processing algorithms, such as the ones that lie at the heart of RTSTFT: the phase vocoder, and the Short-Time Fourier Transform upon which it depends.

<!-- $\displaystyle \int_{-\infty}^{\infty}$  -->

#### The Short-Time Fourier Transform

The Short-Time Fourier Transform (STFT) is a method of extracting data about how the phase and frequency distribution of a signal changes over time. This is achieved by taking overlapping sections of an incoming signal, which is called *windowing*, and performing a Fourier Transform on each of these windowed frames. Practically, this is generally done using a Fast Fourier Transform (FFT), which is an optimized version of the normal DFT that takes advantage of symmetries in the math that can be ignored when working with real-valued input signals. 

The STFT is often performed on overlapping frames of signal, with the factor of the overlap $F_{overlap}$ determining how much the samples frames overlap. This overlap distance, defined as the frame size $N$ divided by $F_{overlap}$, is referred to as the "hop," as it is the distance the algorithm hops to get from one frame to the next. As an example, an STFT with $F_{overlap} = 4$ and $N = 512$ would take a 512-sample frame from the input signal every 128 samples. 

These overlapping frames will also have a *windowing function* applied to them. This reduces the power of samples at the edges of the frames, and tends to increase the precision of the STFT. 

##### Definitions 
Starting with a chosen FFT frame size $N$,  take the following definitions:
$$\text{frame size} = N\hspace{5mm}\text{frame count}=m $$
$$\text{overlap factor}=F_{overlap} \hspace{5mm} \text{scaling ratio}=S$$
$$\text{input sequence}     = x[n]\hspace{5mm}\text{output sequence}    = y[n]$$
$$\text{analysis hop}       = hop_a = \frac{N}{F_{overlap}}$$
$$\text{analysis hop}       = hop_s = \text{round}({S * hop_a})     $$

#### Calculations 
The equation for the output is:
$$y[n] = \sum_{i=0}^{m-1}=O_{i,(n - hop_s)} * (u[n - i * hop_s] - u[n - i * hop_s - N])$$

where $m$ is the number of frames, $O$ is the two-dimensional array of processed FFT frames, and $u[n]$ is the unit step function, defined as:
$$u[n] = \begin{cases} 1 & n \ge 0 \\ 0 & n < 0 \end{cases}$$

## Implementation
### Overview
![image](implementation.png)

### rt_cmd

#### syntax

Limit the amplitude of bins 25 through 60 (inclusive) to $\pm0.5$, where amplitude is measured in the normal DSP convention of $[-1,1]$:

`limit 25-60 0.5`

Apply (roughly) the same limiting using decibels (dBFS):

`limit 25-60 -6`

Apply gain to bins 200-400 utilizing an exponential curve defined by $x^{10^{\text{input}}}$, where $\text{input}\in[-1,1]$ and $\text{input} = 0.5$:

`gain 200-400 -c 0.5 -12 -6`
