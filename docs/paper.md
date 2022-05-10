---
title: THE RTSTFT FRAMEWORK
author: Jackson Kaplan
---

# Abstract
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

# Background & History
## Motivation and philosophy behind creation
- Digital audio is stuck trying to emulate and outperform analog gear
- Very little emphasis on exploring the creative possibilities that are unique to digital sound processing algorithms
- Few spectral plugins that aren't extremely nebulous about their actual internal function
- Very few creative plugins in general that emphasize surgical precision
- Absolutely no spectral plugins that allow direct interface with their underlying algorithms (to my knowledge)
  - RTSTFT is designed for user-level manipulation of individual FFT bins
- Also intended as a lightweight and friendly introduction to command-line syntax
  - A skill I think everyone deserves to be taught but lacks easily available resources for beginners
## History of spectral audio
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


# Mathematics

For this section explaining the math behind RTSTFT, I will be using *extremely* explicit notation. For any readers with a background in maths, it will likely seem like overkill, but I do so in order to create an explanation that I wish had existed when I was learning all of this on my own. The following explanation is intended for a anyone with only a fuzzy recollection of highschool level calculus, and insists upon instilling the reader with a deep understanding of the actual mechanisms and reasoning behind the math instead of just the computations alone.

## An Overview of the Fourier Transform

For some set of $N$ input values $x_{input}$, the Discrete Fourier Transform (DFT) will return an array of values $x_{output}$ at an index $k$ defined as\footnote{For those who may not know: the sigma symbol $\sum$ simply indicates adding up the values of an expression for all integers in the given range. As such, $\sum_{n=0}^{N - 1}f(n)$ simply indicates determining the output of $f(n)$ for all $n$ from $0$ to $N -1$, and then summing all these values together.}:

$$\displaystyle x_{output}[k] = \sum_{n=0}^{N - 1}{x_{input}[n] \cdot e^{\frac{i2\pi kn}{N}}}$$ 


Note that this set of input values, or "signal" as it will be hereon referred to, is zero-indexed, hence taking the sum from 0 to N-1. This equation may certainly seem a little daunting, but we can break it down a bit by noting that it shares the form of Euler's formula, $e^{ix} = \cos{x} + i\sin{x}$. Applying this, we get the following\footnote{Wikipedia}:

$$\displaystyle x_{output}[k] = \sum_{n=0}^{N - 1}{x_{input}[n] \cdot (\cos(\frac{i2\pi kn}{N}) + i\sin(\frac{i2\pi kn}{N}))}$$

Though this seems no less complicated, it tells us something very fundamental about the equation: it is returning a value that lies on some circle where the x-axis is real and the y-axis is imaginary. The DFT imagines the input signal as some addition of many different harmonic sine waves, where the lowest frequency sinusoid has a frequency of 1/N samples, and the highest a frequency of 2 samples. The inner portion of the equation, $x[n] \cdot \frac{i2\pi kn}{N}$, can be interpreted as the complex "responase" of a given frequency $\frac{1}{k}$ at a given sample $n$, with the sum of all these complex numbers representing the response

What matters is, when we take the absolute value and the angle of a complex number at some index $k$ of $x_{output}$ (corresponding to taking the radius and the angle of the complex number on its imaginary circle), the values we get back are the amplitude and the phase of the sinusoid with the frequency $\frac{1}{k}$ samples. 

Let me state that again in simpler terms, for emphasis: ***the DFT decomposes any input signal into a set of sinusoid waves, and gives us the phase and amplitude of each of those sinusoids***. This mathematical operation is not an estimation: it is 100% accurate and completely reversible. Even for people familiar with the DFT, the significance of this cannot be overstated: being able to decompose a signal like this, turning its time-domain information into frequency-domain information, is an invaluable tool in almost any field of mathematics that deals with periodic signals. This explanation still might not suffice for many, as even I still have trouble fully linking the variables in the equation to what's actually going on under the hood. I insist you go to 3blue1brown's YouTube Channel\footnote{3Blue1Brown} and take a look at his video on the DFT; I find the visualizations to be extremely helpful in building a deeper understanding of how the DFT really works.

The DFT lies at the core of countless more complex signal processing algorithms, such as the ones that lie at the heart of RTSTFT: the phase vocoder, and the Short-Time Fourier Transform upon which it depends.

## The Short-Time Fourier Transform

The Short-Time Fourier Transform (STFT) is a method of extracting data about how the phase and frequency distribution of a signal changes over time. This is achieved by taking overlapping sections of an incoming signal, which is called *windowing*, and performing a Fourier Transform on each of these windowed frames. Practically, this is generally done using a Fast Fourier Transform (FFT), which is an optimized version of the normal DFT that takes advantage of symmetries in the math that can be ignored when working with real-valued input signals. 

The STFT is often performed on overlapping frames of signal, with the factor of the overlap $F_{overlap}$ determining how much the samples frames overlap. This overlap distance, defined as the frame size $N$ divided by $F_{overlap}$, is referred to as the "hop," as it is the distance the algorithm hops to get from one frame to the next. As an example, an STFT with $F_{overlap} = 4$ and $N = 1024$ would take a 1024-sample frame from the input signal every 256 samples. A long input signal of 4096 samples, when run through this STFT, would return 13 windowed frames, each sampled 256 samples apart from each other.

These overlapping frames will also have a *windowing function* applied to them \footnote{Götzen et. al.}. This reduces the power of samples at the edges of the frames, and tends to increase the precision of the STFT. The hanning window is the most commonly used of these:

![windowing of a sinusoid](window.png)

Though the STFT is excellent for producing visualization and other analyses of long or real-time signals that would defy easy analysis via a single FFT, their real power becomes apparent when their tiered nature is leveraged to perform complex manipulations on the input signal that would otherwise be impossible.

For a signal that is comprised of $M$ overlapping frames of size $N$, any frame $m \in \{0, ..., M - 1\}$ in the array of analysis frames $A$\footnote{We denote these frames as "analysis" frames as they represented the analyzed amplitude and phase information from the incoming signal. This distinction will be very important when we start modifying the frames using phase vocoding.} is defined by the following equation:

$$A_{m}[k] = (\sum_{n = 0}^{N-1}{
       x_{input}[n] \cdot e^{\frac{i2\pi kn}{N}}) \cdot w[n] 
}$$

for the output fra $x_{frame}$ in the input sequence $x_{input}$, and the windowing function $w[n]$ for the given sample $n$. 

## Phase Vocoder

The phase vocoder is an algorithm that allows for frequency-domain manipulation of signals while keeping the time domain constant. Most likely, you'll know that speeding up audio, like when you change the rotational speed of a record player, changes the pitch. What a phase vocoder can do is change the overall time of a signal while keeping the pitch constant, or change the pitch while keeping the time constant. The way it does this is by ensuring that the phases of each individual sinusoid, or "bin", aligned between the frames. In short, a phase vocoder does this by taking the phase of each bin and adjusting it using the phase of the same bin from the previous frame. 

Time-stretching with an STFT can be accomplished by simply re-layering the windowed frames with a different hop size—e.g., the same 1024/256 STFT mentioned above could make its input audio twice as long by simply re-layering the frames 512 samples apart as opposed to the original 1024. To pitch-shift, one need only resample this audio to its original length; for instance, an audio clip stretched to twice its length could be resampled to its original length to shift its pitch up one octave. 

This introduces some issues. Namely, the STFT does not perfectly account for changes in frequency between frames when said frequency exists somewhere between bins: this is called the "phase coherence" problem \footnote{Srinivas et. al.}. Imagine a very simple STFT that has bins evenly spaced 1Hz apart (we'll assume these frames are exactly 1 second long, for simplicity). If one frame possesses a sinusoid at 1.8Hz, while the subsequent one changes in pitch to 2.2Hz, both of these sinusoids will be "detected" by the 2Hz bin. The issue arises from the fact that the 2Hz bin "assumes" a perfect 2Hz sinusoid, which will fail to accurately represent the phase difference between the two frames if the distance between them is changed, such as in a time-stretch/pitch-shift operation. 

This is where the phase vocoder comes in. Using the phase information from the current bin and the previous bin, it calculates the "true" frequency that is being detected by each bin, which eliminates the phase coherence problem \footnote{Ibid.}.

### Phase Vocoder - Mathematical Definitions

Before beginning, allow me to clarify several things that were difficult for me to grasp as someone who didn't have much exposure to academic-level DSP symbology. One of my main goals with RTSTFT was to make these complex-seeming algorithms much more accessible to musicians and other end-users who may be unfamiliar with many of these concepts, and this paper will follow suit in that regard.

In the following equations, there are two symbols that you may find intimidating: the Greek letters $\phi$ and $\omega$ (phi and omega). $\phi$ is used to denote a phase angle, i.e. the current "position" of a waveform, where as $\omega$ is the angular frequency, which is essentially just the cyclic frequency multipled py $2\pi$ radians. For instance, if we look at the following sine wave:

![sine wave](phi_omega.png)

We note that at 0 radians, it begins with its lowest crest, corresponding to the value of $\sin(\frac{3\pi}4)$, which means this sinusoid has a ***phase angle*** of $\frac{3\pi}4$ radians.

$u[n]$ is the unit step function, defined as:

$$u[n] = \begin{cases} 1 & n \ge 0 \\ 0 & n < 0 \end{cases}$$

In this context, the unit step function is simply used to excluded any of the frames that aren't overlapping the current frame $m$.

## Definitions 
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

# Implementation
## Overview
![image](implementation.png)

## rt_cmd

rt_cmd is the native command line language built directly into the RTSTFT library. Unforunately, it is not nearly as fleshed out as I would have like it to have been, but as I developed the rtstft_ctl plugin, I began to realize how most operations are much simpler to undertake from the GUI as opposed to the command line, especially in the context of music production. That said, I did leave in a command line so that anyone who felt the need to get extremely surgical with rtstft_ctl may do so by leveraging the rt_cmd language.

### syntax

rt_cmd follows an exceptionally simplistic syntactic structure, which can be summed up as:
```
COMMAND [FLAGS[FLAG ARGUMENTS...]...] COMMAND ARGUMENTS...
```


Limit the amplitude of bins 25 through 60 (inclusive) to $\pm0.5$, where amplitude is measured in the normal DSP convention of $[-1,1]$:

`limit 25-60 0.5`

Apply (roughly) the same limiting using decibels (dBFS):

`limit 25-60 -6`

Apply gain to bins 200-400 utilizing an exponential curve defined by $x^{10^{\text{input}}}$, where $\text{input}\in[-1,1]$ and $\text{input} = 0.5$:

`gain 200-400 -c 0.5 -12 -6`

## Future Optimizations

Utilizing the PFFFT Library, RTSTFT runs extremely fast due to the library's usage of both ARM Neon as well as x86 SSE/AVX SIMD intrinsics. For those unfamiliar, SIMD (Single Instruction Multiple Data) instructions are implemented in some processors to be able to perform a single operation on multiple pieces of data, for instance multiplying 8 floating point numbers together in pairs. RTSTFT's algorithm (outside of PFFFT) has not been implemented to take advantage of such instruction sets.

Most of the amplitude manipulations involve a conditional check for every single bin (e.g., the gate manipulation checks if the bin amplitude is less than the corresponding gate value before setting the bin amplitude to 0). This could be implemented in SIMD with commands such as ARM's Neon `VCLE` instruction, or the SSE `mm_cmple_ps` instruction, followed by SIMD multiply; the limit manipulation could be even simpler with Neon's `VMIN` or SSE's `mm_min_ps`.

It may be possible to vectorize the linear interpolation step of the RTSTFT algorithm, but it would likely be prohibitively difficult to implement compared to the small boost in performance it would gain. 

One of RTSTFT's biggest issues at the moment is its horrific worst-case processing performance: while ingesting samples, RTSTFT runs extremely quickly, but the moment there are enough samples to create a full FFT frame, the *entire* phase vocoder algorithm is executed in one fell swoop. For larger FFT sizes, this could very easily halt the audio thread as the processor struggles to finish the computations in time. There are several possible remedies for this:

1. Execute the phase vocoder algorithm on a worker thread whenever an FFT frame is complete. 
2. Subdivide the phase vocoder algorithm into separate chunks of work to be executed one-by-one during different cycles. 

Though the threaded option it is the best solution on paper, it would increase the necessary complexity of RTSTFT exponentially, as managing the synchronization of the worker with the audio thread would involve an extremly fault-tolerant system of checks and fallbacks. The second option, though still requiring an increase in complexity, would save on implementation time as it doens't introduce a threadsafety concern. For instance, once an FFT frame is ready to be read, it could be digested and forward-transformed during once cycle, manipulated during the second cycle, phase vocoded during the third, and then inverse transformed and marked as ready for reading during the fourth. These operations would all still occur sequentially, keeping the DSP of RTSTFT self-contained. At current, this worst-case performance problem only seems to be an issue with FFT sizes of 8192 or higher, most of which can be ameliorated by the user by utilizing offline rendering to ensure a clean signal. 

In all liklihood, all of these optimizations would only be pursued if RTSTFT were ever rewritten with the intent of creating a commercial product, but I will leave them here for any in the open source community who would like to create a higher-performance fork of RTSTFT.

\pagebreak
# Bibliography 

1. 3Blue1Brown. (2018, January 26). But what is the Fourier Transform?  A visual introduction. https://www.youtube.com/watch?v=spUNpyF58BY
2. Discrete Fourier transform. (2022). In Wikipedia. https://en.wikipedia.org/w/index.php?title=Discrete_Fourier_transform&oldid=1081065184
3. Fourier transform. (2022). In Wikipedia. https://en.wikipedia.org/w/index.php?title=Fourier_transform&oldid=1085783068
4. Götzen, A., Arfib, D., & Bernardini, N. (2000). Traditional(?) implementation of a phase vocoder: The tricks of the trade. 37–44.
5. Lim, K. A. (n.d.). An Open-Source Phase Vocoder with Some Novel Visualizations. 39.
6. Srinivas, N., Amara, M., & Kumar, P. K. (n.d.). Implementation of Pitch Shifter using Phase Vocoder Algorithm on Artix-7 FPGA. 8.
7. The Mathematical Genius of Auto-Tune. (2016, September 26). Priceonomics. https://priceonomics.com/the-inventor-of-auto-tune/