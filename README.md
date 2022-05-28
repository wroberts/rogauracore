# rogauracore - RGB keyboard control for Asus ROG laptops

(c) 2019 Will Roberts

Linux-compatible open-source libusb implementation similar to the ROG
Aura Core software.  Supports RGB keyboards with IDs
[0b05:1854](https://linux-hardware.org/index.php?id=usb:0b05-1854)
(GL553, GL753),
[0b05:1869](https://linux-hardware.org/index.php?id=usb:0b05-1869)
(GL503, FX503, GL703), [0b05:1866](https://linux-hardware.org/index.php?id=usb:0b05-1866) (GL504, GL533, GL703, GX501, GM501), and [0b05:19b6](https://linux-hardware.org/index.php?id=usb:0b05-19b6) (GA503).

## Usage

```
Usage:
   rogauracore COMMAND ARGUMENTS

COMMAND should be one of:
   single_static
   single_breathing
   single_colorcycle
   multi_static
   multi_breathing
   rainbow_cycle
   red
   green
   blue
   yellow
   gold
   cyan
   magenta
   white
   black
   rainbow
   brightness
   initialize_keyboard
```

In typical use, you will need root privileges to directly communicate
with the laptop's keyboard.  This is easy to do with `sudo`.  Try some
of these commands and see what works for you:

```sh
sudo rogauracore single_static 0000ff
sudo rogauracore single_static 00ff00
sudo rogauracore single_static ffff00
sudo rogauracore multi_static ff0000 ffff00 00ff00 00ffff
sudo rogauracore single_colorcycle 1
```

If your keyboard does not respond to `rogauracore`, it may help to
send an initialisation message to the keyboard to "wake it up":

```sh
sudo rogauracore initialize_keyboard
```

If your keyboard remains dark, its brightness might have defaulted to 0. Try:

```sh
sudo rogauracore brightness 3
```

## Installation

### Ubuntu

In all cases you will need `libusb` and `libusb-dev` installed:
```sh
sudo apt install libusb-1.0-0 libusb-1.0-0-dev
```
Optionally you might also need build tools:
```sh
sudo apt install build-essential
```

Retrieve the `rogauracore` package, either through `curl` or `git`:
```sh
VERSION=1.6
curl -LOs https://github.com/wroberts/rogauracore/releases/download/$VERSION/rogauracore-$VERSION.tar.gz
tar xf rogauracore-$VERSION.tar.gz
cd rogauracore-$VERSION/
```
or
```sh
git clone https://github.com/wroberts/rogauracore.git
cd rogauracore
autoreconf -i
```

Then configure, make and install:
```sh
./configure
make
sudo make install
```

## Tips and tricks

On some machines, running `rogauracore` can cause the system's power
management to not be able to find the keyboard backlight control.
This manifests as unresponsive UI controls for brightening and
darkening the keyboard backlight.  If this issue affects you,
[@willlovesbearz](https://github.com/willlovesbearz) suggests running
this command after `rogauracore`:

```sh
sudo systemctl restart upower.service
```

## Related projects

- [rogauracore-gui](https://github.com/rastafaninplakeibol/rogauracore-gui)
  is an Electron-based GUI for `rogauracore` by
  [@rastafaninplakeibol](https://github.com/rastafaninplakeibol).

- [OpenRGB](https://gitlab.com/CalcProgrammer1/OpenRGB) can probably
  do all the things that `rogauracore` can do.
