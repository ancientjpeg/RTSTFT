# (jk's) RTSTFT

### \*\***PLEASE NOTE**\*\*
This library is very incomplete.

## Synopsis
-----

RTSTFT is (or will be, at least), a library written in pure ANSI C that will lie at the core of jk's as-of yet unnamed audio plug-in. At its core, it is a framework that allows for extremely precise manipulation of the Short-Time Fourier Transform, a function that lies at the core of much of today's spectral-based audio processors. 

## TODO
-----
- [x] Data structures capable of handling audio blocks of unpredictable size
- [x] Algorithm performs an unmodified STFT and returns an accurate result
- [ ] Phase vocoder pitch-shifting (half-complete; sounds wrong below FFT size of ~1024-2048)
- [ ] Data structures capable of resizing FFT on the fly
- [ ] Per-bin spectral gating and muting
- [ ] Small scripting language for precise per-bin control
- [ ] ???
- [ ] success

## Compilation
-----

At current, compilation requires an up-to-date copy of the FFTW3 library to be placed in a directory called fftw_src. For now, simply grab the tarball from [their site](https://www.fftw.org/download.html),
extract it to your local RTSTFT repository, and rename it to fftw_src. From there, the Makefile should *hopefully* handle everything! Just cd to the repo, and execute the following:

```make
make
```

**please note:**
At the moment, this will generate an executable, not a static library, for the time being. The executable is based on tests.c, which is a small test program for the library.The test executable also requires an "in.wav" file to be present in the directory. This MUST be a bare 16-44.1k .wav with no metadata in the header beyond the requisite 44 byte WAV header. 

<span style="text-decoration: underline">_**This will be changed in the future.**_</span>

## Basic Usage
-----

The following code will intialize an RTSTFT instance with 2 channels, an FFT frame size of 512 samples, a maximum input buffer size of 2048 samples, an STFT overlap factor of 4, at a sampling rate of 44,100 kHz:


```c
rt_params params = rt_init(2, 512, 2048, 4, 0, 44100);
rt_real *buffers[num_channels];
/* assign some data to the buffers */
rt_cycle(p, buffers, num_channels, 64);

```
After the above block of code, `rt_cycle` will have consumed 64 samples of data but output 64 zeros back to the buffers. RTSTFT is a real-time algorithm (it's in the name!), so it will write back zeros until it has fully processed its first FFT frame, indicating a latency equal to the current FFT frame size.

When you're done, you can tidily deallocate all the memory RTSTFT used with a single line of code:
```c
rt_clean(params);
```

## Advanced Usage and Configuration
-----

#### Compilation

**I still have not thoroughly tested the Makefile!** Though I feel it is extraordinarily unlikely that a `make` error could cause any damage outside the local directory in the event of an error, I have only tested it on my personal machine so far.

To enable RTSTFT in double-precision, just define the RT_DOUBLE environment variable in your make command:
```make
make RT_DOUBLE=1
```

If you would like to customize your installation of FFTW, you can find the relevant documentation on their site [here](https://www.fftw.org/fftw3_doc/Installation-and-Customization.html). In the context of RTSTFT, you just need to cd into the fftw_src directory *__before__* running `make`, and simply compile FFTW yourself following their instructions, i.e.:
```make
./configure [YOUR FFTW CONFIG FLAGS HERE]
make
make install
```
As of right now, I cannot make any promises as to whether or not any of these changes will break RTSTFT. In particular, enabling threading on FFTW could theoretically cause some issues––it won't *necessarily* break anything, but I have not prepared the RTSTFT system to handle FFTW threading its allocation routines.

#### Data Types
To use RTSTFT, it is highly recommended to place your data into arrays of type `rt_real`, which is a floating-point type that is defined to allow the library to be implemented in single or double precision. If this is not possible, you can verify the floating-point precision using the provided utility function `rt_real_size()`.

`rt_uint` is an unsigned integral type that is guaranteed at compile time to be at least 4 bytes. This is done for internal consistency, and for most practical purposes one needn't worry about this and may simply pass integer literals (as long as they're not negative!).

`rt_params` is an opaque pointer type used to communicate and manage RTSTFT algorithm's state. This typedef was not made for the sake of convenience, but instead to clearly convey that the internal workings of RTSTFT should not be meddled with directly at runtime. Instead, RTSTFT implements an outward-facing API where each function takes an `rt_params` variable as its first argument.

#### API
With its simplest API functionality, RTSTFT will take blocks of data from some number of channels, digest and process the data using its own internal data structures, and write its results to the same buffer it read from. One must first initialize an instance of `rt_params`, which should always be done using the API's `rt_init` function.

Let's take a look at `rt_init`:
```c
rt_params rt_init(rt_uint num_channels, rt_uint frame_size, 
                  rt_uint buffer_size, rt_uint overlap_factor, 
                  rt_uint pad_factor, float sample_rate);
```

The first argument to `rt_init` is the number of channels, the second is the FFT frame size to be used.

The third argument is the *maximum* amount of samples you intend to send it during any given cycle. `rt_cycle` is capable of accepting a buffer of any size, but the program will crash if you go above this prescribed maximum size as the RTSTFT instance may not have allocated enough memory to do so. The only exception is if the max buffer size is less than double the FFT frame size, in which case RTSTFT will take the latter as its maximum buffer size *(I know this is nonsense. I'll fix it later)*.



The following argument is the STFT overlap factor, which is equal to the FFT frame size divided by the STFT hop size. 

The penultimate argument is the zero-padding factor, which will scale the size of the FFT by 2<sup>zero-pad</sup> for increased frequency resolution. The final argument is the current sampling rate.

After this, `rt_cycle` will happily consume all the data you feed it and spit out the processed result. **HOWEVER,** this will not work if you point rt_cycle at the same block of data. `rt_cycle` assumes that every buffer it is fed is the *next* buffer in a sequence of data, which is common in real-time DSP applications where the audio host will send in new blocks of audio data once one block has been processed. If you'd like to run RTSTFT on a long, contiguous block of data, you have two options:

##### 1. Increment your own buffer pointers

This solution is simple and may be preferred by some. Between each call to `rt_cycle`, you may simply adjust your own buffer pointers so they point to the next blocked of *unprocessed* data.


##### 2. Use rt_cycle_offset()

We provide a convenience function `rt_cycle_offset()`, which will offset RTSTFT's read position by the desired number of samples.

```c
/* buffers contains 2 channels of 65536 samples */
int samples_read = 0;
int read_size = 64
while (samples_read < 65536) {
    rt_cycle_offset(p, buffers, num_channels, samples_read);
    samples_read += read_size;
}
```

What's nice about this is that read_size can actually be any size, up to the maximum buffer size. You can feed RTSTFT a random number of samples during any cycle and it'll chug along happily. **I HAVE NOT TESTED THIS STATEMENT YET.**

# License

Undecided how I'll got about this, but for the meantime:

Copyright (C) Jackson Kaplan - All Rights Reserved
Unauthorized copying of this file, via any medium is strictly prohibited
Proprietary and confidential
Developed by Jackson Kaplan in 2022
Most recent update: 2022-02-04


(maybe I'll go open-source later)



