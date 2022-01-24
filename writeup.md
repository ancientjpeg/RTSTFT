## DEFINITIONS
---
We start with a chosen FFT frame size $N$ 
Take the following variable definitions:
$\text{frame size} = N\hspace{5mm}\text{frame count}=m$
$$\text{overlap factor}=F_{overlap} \hspace{5mm} \text{scaling ratio}=S$$
$$\text{input sequence}     = x[n]\hspace{5mm}\text{output sequence}    = y[n]$$
$$\text{unit step function} = u[n]$$
$$\text{analysis hop}       = hop_a = \frac{N}{F_{overlap}}$$
$$\text{analysis hop}       = hop_s = \text{round}({S * hop_a})     $$
The equation for the output is:
$$y[n] = \sum_{i=0}^{m-1}=O_{i,(n - hop_s)} * (u[n - i * hop_s] - u[n - i * hop_s - N])$$

where $m$ is the number of frames, $O$ is the two-dimensional array of processed FFT frames, and $u[n]$ is the unit step function, defined as:
$$u[n] = \begin{cases} 1 & n \ge 0 \\ 0 & n < 0 \end{cases}$$