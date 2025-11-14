  ----------- ------------------------- -----------
  AUDIOV(1)   General Commands Manual   AUDIOV(1)
  ----------- ------------------------- -----------

::::::::::::::::: manual-text
:::: {.section .Sh}
# [NAME](#NAME){.permalink} {#NAME .Sh}

`audiov`{.Nm} ---

::: Nd
graphical audio frequency visualizer
:::
::::

::: {.section .Sh}
# [SYNOPSIS](#SYNOPSIS){.permalink} {#SYNOPSIS .Sh}

  --------------- -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  `audiov`{.Nm}   \[`-c`{.Fl} `channels`{.variable .Ar}\] \[`-d`{.Fl} `device`{.variable .Ar}\] \[`-e`{.Fl} `encoding`{.variable .Ar}\] \[`-f`{.Fl} `fft-samples`{.variable .Ar}\] \[`-m`{.Fl} `fft-min`{.variable .Ar}\] \[`-p`{.Fl} `precision`{.variable .Ar}\] \[`-s`{.Fl} `sample-rate`{.variable .Ar}\] \[`-C`{.Fl} `color`{.variable .Ar}\] \[`-E`{.Fl} `color-end`{.variable .Ar}\] \[`-H`{.Fl} `box-height`{.variable .Ar}\] \[`-M`{.Fl} `milliseconds`{.variable .Ar}\] \[`-N`{.Fl} `num-bars`{.variable .Ar}\] \[`-S`{.Fl} `box-space`{.variable .Ar}\] \[`-U`{.Fl}\] \[`-X`{.Fl}\] \[`-W`{.Fl} `bar-width`{.variable .Ar}\]
  --------------- -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
:::

::: {.section .Sh}
# [DESCRIPTION](#DESCRIPTION){.permalink} {#DESCRIPTION .Sh}

`audiov`{.Nm} is a [curses(3)](../html3/curses.html){.Xr} frontend to
test [NetBSD]{.Ux} [audio(4)](../html4/audio.html){.Xr} recording
devices.

`audiov`{.Nm} continually captures input from the `device`{.variable
.Ar} and renders visualizations of that data in the frequency domain

The following options are available:

[`-c,`{#c, .Fl}](#c,){.permalink} `--channels`{.Fl} `channels`{.variable .Ar}
:   The number of channels to use with the recording device. Defaults to
    the preconfigured value for the device.

[`-d,`{#d, .Fl}](#d,){.permalink} `--device`{.Fl} `device`{.variable .Ar}
:   The recording audio device. Write access to the device is required.
    Defaults to /dev/sound.

[`-e,`{#e, .Fl}](#e,){.permalink} `--encoding`{.Fl} `encoding`{.variable .Ar}
:   The encoding to use with the recording device. The following options
    are available: ulinear, ulinear_le, ulinear_be, slinear, slinear_le,
    slinear_be. Defaults to the preconfigured value for the device.

[`-f,`{#f, .Fl}](#f,){.permalink} `--fft-samples`{.Fl} `fft-samples`{.variable .Ar}
:   The number of samples to use for each fast fourier transform. The
    number of samples configures the precision of the fourier transform
    (bins = samples / 2). Defaults to 1024.

[`-m,`{#m, .Fl}](#m,){.permalink} `--fft-min`{.Fl} `fft-min`{.variable .Ar}
:   The starting frequency for the first bar of the visualization.
    Defaults to 50.

[`-p,`{#p, .Fl}](#p,){.permalink} `--precision`{.Fl} `precision`{.variable .Ar}
:   The bit precision of each sample. Defaults to the preconfigured
    value for the device.

[`-s,`{#s, .Fl}](#s,){.permalink} `--sample-rate`{.Fl} `sample-rate`{.variable .Ar}
:   The sample rate of the device. Determines the max frequency of fast
    fourier transform (fmax = sample-rate / 2). Defaults to the
    preconfigured value for the device.

[`-C,`{#C, .Fl}](#C,){.permalink} `--color`{.Fl} `color`{.variable .Ar}
:   The color of each bar. By default color mode is disabled. Specifing
    the color automatically enables color mode so -U does not have to be
    explicitly added.

[`-E,`{#E, .Fl}](#E,){.permalink} `--color-end`{.Fl} `color-end`{.variable .Ar}
:   The end color of each bar. If specified, each bar will transition
    from color to color-end as the magnitude increases. color-end will
    be ignored unless box mode (-X) is enabled and color (-C) are
    specified.

[`-H,`{#H, .Fl}](#H,){.permalink} `--box-height`{.Fl} `box-height`{.variable .Ar}
:   Specifies the height of each box in a bar. Will be ignored unless
    box mode (-X) is enabled. Defaults to 2.

[`-M,`{#M, .Fl}](#M,){.permalink} `--milliseconds`{.Fl} `milliseconds`{.variable .Ar}
:   The duration of recorded audio captured every interval. Defaults to
    150.

[`-N,`{#N, .Fl}](#N,){.permalink} `--num-bars`{.Fl} `num-bars`{.variable .Ar}
:   The number of bars to render. Defaults to a computation based on the
    configured bar-width and the size of the screen.

[`-S,`{#S, .Fl}](#S,){.permalink} `--box-space`{.Fl} `box-space`{.variable .Ar}
:   Specifies the amount of space between each box of a bar. Will be
    ignored unless box mode (-X) is enabled. Defaults to 1.

[`-U,`{#U, .Fl}](#U,){.permalink} `--use-colors`{.Fl}
:   Enables color mode. Each bar will be filled in using the system\'s
    default text color, unless overridden by specifying a color (-C).

[`-X,`{#X, .Fl}](#X,){.permalink} `--use-boxes`{.Fl}
:   Enables box mode. When enabled, each bar is broken into discrete
    boxes, each of size box-height (-H), separated by box-space (-S).
:::

::: {.section .Sh}
# [COLORS](#COLORS){.permalink} {#COLORS .Sh}

`audiov`{.Nm} can render colors if `color`{.variable .Ar} +
`color-end`{.variable .Ar} are specified. The following options are
available:

black, red, yellow, blue, magenta, cyan, white
:    
:::

::: {.section .Sh}
# [NAVIGATION](#NAVIGATION){.permalink} {#NAVIGATION .Sh}

`audiov`{.Nm} uses the following keys for navigation:

V
:   View the Frequency domain of the recorded audio.

I
:   View all configuration details. Can use j/k to scroll.

Q
:   Exit the application.
:::

:::::::: {.section .Sh}
# [EXAMPLES](#EXAMPLES){.permalink} {#EXAMPLES .Sh}

::: {.Bd .Bd-indent}
audiov -d /dev/audio2
:::

::: {.Bd .Bd-indent}
audiov -U -W 8
:::

::: {.Bd .Bd-indent}
audiov -C red -M 100
:::

::: {.Bd .Bd-indent}
audiov -X -C cyan -E blue
:::

::: {.Bd .Bd-indent}
audiov -X -C red -E green -S 0
:::
::::::::

::: {.section .Sh}
# [SEE ALSO](#SEE_ALSO){.permalink} {#SEE_ALSO .Sh}

[audio(4)](../html4/audio.html){.Xr}
[audiocfg(1)](../html1/audiocfg.html){.Xr}
:::

::: {.section .Sh}
# [AUTHORS](#AUTHORS){.permalink} {#AUTHORS .Sh}

`audiov`{.Nm} was written by [Zac Brown]{.An}
⟨gummybuns@protonmail.com⟩.
:::
:::::::::::::::::

  ---------------- -------------
  April 18, 2025   NetBSD 10.1
  ---------------- -------------
