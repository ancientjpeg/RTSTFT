# (jk's) RTSTFT

### Synopsis

RTSTFT is (or will be, at least), a C library that will lie at the core of jk's as-of yet unnamed audio plug-in. At its core, it is simply a Short-Time Fourier transform that is designed to be run at audio rate with optimizations for latency. RTSTFT will also maintain functionality for manipulating the FFT bands further (e.g. band shifting, pitch shifting, etc.), while also outwardly presenting some of its parameters to be manipulated by the plug-in (e.g. allowing the plug-in user to selectively gate some of the FFT bands).

###### PLEASE NOTE
This library is completely non-functional as of right now.
