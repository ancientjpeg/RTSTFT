# THE RTSTFT FRAMEWORK

## Abstract
- What is it?
  - Pure C Library
  - A core STFT algorithm with several methods for manipulation
    - Per-bin manipulations, i.e. gating, limiting, muting
    - Phase-vocoder based pitch-shifting
    - Freeform phase manipulation for effects akin to phase chorus
    - More to come...
  - A scripting language, rt_cmd, for precise control of these parameters
    - Includes a custom command parser

## Background & History
- Motivation and philosophy creation
- 

## Mathematics
#### Definitions 
Starting with a chosen FFT frame size $N$,  take the following definitions:
$$\text{frame size} = N\hspace{5mm}\text{frame count}=m $$
$$\text{overlap factor}=F_{overlap} \hspace{5mm} \text{scaling ratio}=S$$
$$\text{input sequence}     = x[n]\hspace{5mm}\text{output sequence}    = y[n]$$
$$\text{unit step function} = u[n]$$
$$\text{analysis hop}       = hop_a = \frac{N}{F_{overlap}}$$
$$\text{analysis hop}       = hop_s = \text{round}({S * hop_a})     $$

#### Calculations 
The equation for the output is:
$$y[n] = \sum_{i=0}^{m-1}=O_{i,(n - hop_s)} * (u[n - i * hop_s] - u[n - i * hop_s - N])$$

where $m$ is the number of frames, $O$ is the two-dimensional array of processed FFT frames, and $u[n]$ is the unit step function, defined as:
$$u[n] = \begin{cases} 1 & n \ge 0 \\ 0 & n < 0 \end{cases}$$

## rt_cmd

#### syntax


`limit 25-60 0.5`

`limit 25-60 -6`

Apply gain utilizing a quadratic curve
`gain 200-400 -c 0.5 -12 -6`
