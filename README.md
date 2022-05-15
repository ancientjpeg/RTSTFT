# (jk's) RTSTFT

### \*\***PLEASE NOTE**\*\*

This library is very incomplete.

## Synopsis

---

RTSTFT is (or will be, at least), a C library that will lie at the core of jk's as-of yet unnamed audio plug-in. At its core, it is a framework that allows for extremely precise manipulation of the Short-Time Fourier Transform, a function that powers much of today's DSP.

## TODO

---

- [x] Data structures capable of handling audio blocks of unpredictable size
- [x] Algorithm performs an unmodified STFT and returns an accurate result
- [x] Phase vocoder pitch-shifting
- [ ] Data structures capable of resizing FFT on the fly
  - [ ] this may not be practical...
- [ ] Per-bin spectral gating and muting
  - [x] Create storage structure
  - [x] Create logic to apply manipulations
  - [ ] Create functions to adjust manips
  - [ ] Manage manips in sync with control rate
  - [ ]
- [ ] Small scripting language for precise per-bin control
- [ ] ???
- [ ] success

## Compilation

---

At current, compilation requires an up-to-date copy of the FFTW3 library to be placed in a directory called fftw_src. For now, simply grab the tarball from [their site](https://www.fftw.org/download.html),
extract it to your local RTSTFT repository, and rename it to fftw_src. From there, the Makefile should _hopefully_ handle everything! Just cd to the repo, and execute the following:

```make
make
```

**please note:**
At the moment, this will generate an executable, not a static library, for the time being. The executable is based on tests.c, which is a small test program for the library.The test executable also requires an "in.wav" file to be present in the directory. This MUST be a bare 16-44.1k .wav with no metadata in the header beyond the requisite 44 byte WAV header.

<span style="text-decoration: underline">_**This will be changed in the future.**_</span>

## Basic Usage

---

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

---

#### Compilation

**I still have not thoroughly tested the Makefile!** Though I feel it is extraordinarily unlikely that a `make` error could cause any damage outside the local directory in the event of an error, I have only tested it on my personal machine so far.

#### Data Types

To use RTSTFT, it is highly recommended to place your data into arrays of type `rt_real`, which is a floating-point type that is defined to allow the library to be implemented in single or double precision. The capability for switching RTSTFT into double precision was lost when FFTW was dropped in favor of PFFFT, but this may return in the future, thus `rt_real` has been kept. You can verify the floating-point precision using the provided utility function `rt_real_size()`.

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

The third argument is the _maximum_ amount of samples you intend to send it during any given cycle. `rt_cycle` is capable of accepting a buffer of any size, but the program will crash if you go above this prescribed maximum size as the RTSTFT instance may not have allocated enough memory to do so. The only exception is if the max buffer size is less than double the FFT frame size, in which case RTSTFT will take the latter as its maximum buffer size _(I know this is nonsense. I'll fix it later)_.

The following argument is the STFT overlap factor, which is equal to the FFT frame size divided by the STFT hop size.

The penultimate argument is the zero-padding factor, which will scale the size of the FFT by 2<sup>zero-pad</sup> for increased frequency resolution. The final argument is the current sampling rate.

After this, `rt_cycle` will happily consume all the data you feed it and spit out the processed result. **HOWEVER,** this will not work if you point rt_cycle at the same block of data. `rt_cycle` assumes that every buffer it is fed is the _next_ buffer in a sequence of data, which is common in real-time DSP applications where the audio host will send in new blocks of audio data once one block has been processed. If you'd like to run RTSTFT on a long, contiguous block of data, you have two options:

##### 1. Increment your own buffer pointers

This solution is simple and may be preferred by some. Between each call to `rt_cycle`, you may simply adjust your own buffer pointers so they point to the next blocked of _unprocessed_ data.

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

#### Specifications and quirks

- The rt_parser API is **not** threadsafe: it is the user's job to ensure that any call to rt_parser is complete before another call is made. This is because the command buffer automatically clears itself at the beginning of each parsing call, which could result in a race condition deleting the internal state of the parser while it's still executing the command. I may remedy this later, but for now it is easily dealt with by using a mutex in your application's caller.

## License

```
RTSTFT: a phase-vocoder and audio manipulation library
Copyright (C) 2022  Jackson Kaplan

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
```

### PFFFT License

```
Copyright (c) 2013  Julien Pommier ( pommier@modartt.com )

Based on original fortran 77 code from FFTPACKv4 from NETLIB
(http://www.netlib.org/fftpack), authored by Dr Paul Swarztrauber
of NCAR, in 1985.

As confirmed by the NCAR fftpack software curators, the following
FFTPACKv5 license applies to FFTPACKv4 sources. My changes are
released under the same terms.

FFTPACK license:

http://www.cisl.ucar.edu/css/software/fftpack5/ftpk.html

Copyright (c) 2004 the University Corporation for Atmospheric
Research ("UCAR"). All rights reserved. Developed by NCAR's
Computational and Information Systems Laboratory, UCAR,
www.cisl.ucar.edu.

Redistribution and use of the Software in source and binary forms,
with or without modification, is permitted provided that the
following conditions are met:

- Neither the names of NCAR's Computational and Information Systems
Laboratory, the University Corporation for Atmospheric Research,
nor the names of its sponsors or contributors may be used to
endorse or promote products derived from this Software without
specific prior written permission.  

- Redistributions of source code must retain the above copyright
notices, this list of conditions, and the disclaimer below.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions, and the disclaimer below in the
documentation and/or other materials provided with the
distribution.

THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
SOFTWARE.

```