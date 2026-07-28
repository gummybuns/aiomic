# audioviz

`audioviz` is a curses frontend to test NetBSD audio recording devices.


https://github.com/user-attachments/assets/7043a073-6c54-4812-b8b6-984a3bceecb0

## Build

```bash
make
```

## Usage

AUDIOVIZ(1) - General Commands Manual

# NAME

**audioviz** - graphical audio frequency visualizer

# SYNOPSIS

**audioviz**
\[**-c**&nbsp;*channels*]
\[**-d**&nbsp;*device*]
\[**-e**&nbsp;*encoding*]
\[**-f**&nbsp;*fft-samples*]
\[**-m**&nbsp;*fft-min*]
\[**-h**]
\[**-o**&nbsp;*output-device*]
\[**-p**&nbsp;*precision*]
\[**-s**&nbsp;*sample-rate*]
\[**-C**&nbsp;*color*]
\[**-E**&nbsp;*color-end*]
\[**-H**&nbsp;*box-height*]
\[**-M**&nbsp;*milliseconds*]
\[**-N**&nbsp;*num-bars*]
\[**-P**&nbsp;*play-rate*]
\[**-R**&nbsp;*record-rate*]
\[**-S**&nbsp;*box-space*]
\[**-U**]
\[**-W**&nbsp;*bar-width*]
\[**-X**]

# DESCRIPTION

**audioviz**
is a
curses(3)
frontend to test
NetBSD
audio(4)
recording devices.

**audioviz**
continually captures input from the
*device*
and renders visualizations of that data in the frequency domain

The following options are available:

**-c,** **--channels** *channels*

> The number of channels to use with the recording device. Defaults to the
> preconfigured value for the device.

**-d,** **--device** *device*

> The recording audio device. Write access to the device is required. Defaults to
> /dev/audio.

**-e,** **--encoding** *encoding*

> The encoding to use with the recording device. The following options are
> available: ulinear, ulinear\_le, ulinear\_be, slinear, slinear\_le, slinear\_be.
> Defaults to the preconfigured value for the device.

**-f,** **--fft-samples** *fft-samples*

> The number of samples to use for each fast fourier transform. The number of
> samples configures the precision of the fourier transform (bins = samples / 2).
> Defaults to 1024.

**-h,** **--help**

> Prints a help message.

**-m,** **--fft-min** *fft-min*

> The starting frequency for the first bar of the visualization. Defaults to 50.

**-o,** **--output-device** *output-device*

> The audio device to play the recorded audio. The output device will be
> configured to match the same settings as the
> *device.*
> Defaults to /dev/audio when used without an argument.

**-p,** **--precision** *precision*

> The bit precision of each sample. Defaults to the preconfigured value for the
> device.

**-s,** **--sample-rate** *sample-rate*

> The sample rate of the device. Determines the max frequency of fast fourier
> transform (fmax = sample-rate / 2). Defaults to the preconfigured value for the
> device.

**-C,** **--color** *color*

> The color of each bar. By default color mode is disabled. Specifing the color
> automatically enables color mode so -U does not have to be explicitly added.

**-E,** **--color-end** *color-end*

> The end color of each bar. If specified, each bar will transition from color
> to color-end as the magnitude increases. color-end will be
> ignored unless box mode (-X) is enabled and color (-C) are specified.

**-H,** **--box-height** *box-height*

> Specifies the height of each box in a bar. Will be ignored unless box mode (-X)
> is enabled. Defaults to 2.

**-M,** **--milliseconds** *milliseconds*

> The duration of recorded audio captured every interval. Defaults to 150.

**-N,** **--num-bars** *num-bars*

> The number of bars to render. Defaults to a computation based on the configured
> bar-width and the size of the screen.

**-R,** **--record-rate** *record-rate*

> The number of bytes to read from the
> *device*
> at a time. If unspecified, the
> *record-rate*
> will be the minimum between the
> *buffer-size*
> and the number of bytes needed to stream the specified
> *milliseconds*
> worth of data

**-P,** **--play-rate** *play-rate*

> The number of bytes to write to the
> *output-device*
> at a time. If unspecified, the
> *play-rate*
> will be the minimum between the
> *buffer-size*
> and the number of bytes needed to stream the specified
> *milliseconds*
> worth of data

**-S,** **--box-space** *box-space*

> Specifies the amount of space between each box of a bar. Will be ignored unless
> box mode (-X) is enabled. Defaults to 1.

**-U,** **--use-colors**

> Enables color mode. Each bar will be filled in using the system's default text
> color, unless overridden by specifying a color (-C).

**-W,** **--bar-width**

> The width of each individual bar.

**-X,** **--use-boxes**

> Enables box mode. When enabled, each bar is broken into discrete boxes, each of
> size box-height (-H), separated by box-space (-S).

# COLORS

**audioviz**
can render colors if
*color*
\+
*color-end*
are specified. The following options are available:

black, red, yellow, blue, magenta, cyan, white

# NAVIGATION

**audioviz**
uses the following keys for navigation:

v

> View the Frequency domain of the recorded audio.

i

> View all configuration details. Can use j/k to scroll.

q

> Exit the application.

0-9

> While viewing the Frequency domain you can press a number key to filter the
> audio data for that specific channel. Press 0 to select all channels.

# Playback Support

**audioviz**
supports playback of the recorded audio when the
*output-device*
is specified. The
*output-device*
will have its device configuration match the settings of the
*device*
and will throw an error if for some reason that configuration fails to set.
There is no noise/echo reduction and
**audioviz**
is easily susceptible to a painful feedback loop unless the
*output-device*
is headphones or something that prevents the
*device*
from picking up output.

**audioviz**
can support
pad(4)
devices when specified as the
*device*
and playback can then be echo'd when specifying the
*output-device*
as a physical audio device.

> audioviz -d /dev/pad0 -o /dev/audio1

> mpg123 -a /dev/audio2 /path/to/music.mp3

# EXAMPLES

> audioviz -d /dev/audio2

> audioviz -U -W 8

> audioviz -C red -M 100

> audioviz -X -C cyan -E blue

> audioviz -X -C red -E green -S 0

# SEE ALSO

audio(4)
audiocfg(1)
pad(4)

# AUTHORS

**audioviz**
was written by
Zac Brown
&lt;gummybuns@protonmail.com&gt;.

NetBSD 10.1 - July 22, 2026
