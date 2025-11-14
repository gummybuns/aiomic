# AIOMIC

`aiomic` is a curses frontend to test NetBSD audio recording devices.

It continually captures input from the device and measures the Root Mean Square
of the collected samples.

`aiomic` uses the existing configuration of the device and displays the RMS as
a percentage of the maximum value based on the device's precision.

## Build

```bash
make
```

## Usage

```bash
./aioic /dev/audio1
```

### Navigation

`aiomic` uses the following keys for navigation:

- `R` - View the RMS of the recorded audio
- `I` - View the configuration details of the device
- `Q` - Exit the application
