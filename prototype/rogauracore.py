#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Control the RGB keyboard on the Asus ROG Strix GL503VD.

sudo apt install python3-usb

rogauracore.py
(c) Will Roberts  11 March, 2019
"""

from collections import namedtuple
from enum import Enum
import array
import binascii
import inspect
import sys
import usb.core
import usb.util

ASUS_VENDOR_ID = 0x0B05
ASUS_PRODUCT_ID = 0x1869


class Color(namedtuple("Color", ["red", "green", "blue"])):
    __slots__ = ()

    @classmethod
    def fromhexstr(cls, sval):
        return cls(*list(binascii.unhexlify(sval.lstrip("#"))))


class Speed(Enum):
    SLOW = 1
    MEDIUM = 2
    FAST = 3

    @property
    def bytevalue(self):
        return [0xE1, 0xEB, 0xF5][self.value - 1]


def toarray(sval):
    arr = array.array("B")
    arr.frombytes(binascii.unhexlify(sval))
    return arr


SET_MESSAGE = toarray("5db5000000000000000000000000000000")
APPLY_MESSAGE = toarray("5db4000000000000000000000000000000")


def acquire_usb():
    dev = usb.core.find(idVendor=ASUS_VENDOR_ID, idProduct=ASUS_PRODUCT_ID)
    if dev is None:
        print("Could not find ASUS RGB keyboard.")
        sys.exit(1)

    try:
        assert dev.bNumConfigurations == 1
        cfg = dev.configurations()[0]
        assert cfg.bNumInterfaces == 1
        iface = 0

        active_iface = False
        if dev.is_kernel_driver_active(iface):
            dev.detach_kernel_driver(iface)
            active_iface = True
    except usb.USBError as e:
        usb.util.dispose_resources(dev)
        raise e

    try:
        usb.util.claim_interface(dev, iface)
    except usb.USBError as e:
        if active_iface:
            dev.attach_kernel_driver(iface)
        usb.util.dispose_resources(dev)
        raise e

    return dev, iface, active_iface


def release_usb(dev, iface, active_iface):
    usb.util.release_interface(dev, iface)
    if active_iface:
        dev.attach_kernel_driver(iface)
    usb.util.dispose_resources(dev)


def single_static(color):
    arr = toarray("5db30000ff000000000000000000000000")
    arr[4] = color.red
    arr[5] = color.green
    arr[6] = color.blue
    return [arr, SET_MESSAGE, APPLY_MESSAGE]


def single_breathing(color1, color2, speed):
    arr = toarray("5db3000108fff0eb0001ff000000000000")
    arr[4] = color1.red
    arr[5] = color1.green
    arr[6] = color1.blue
    arr[10] = color2.red
    arr[11] = color2.green
    arr[12] = color2.blue
    arr[7] = speed.bytevalue
    return [arr, SET_MESSAGE, APPLY_MESSAGE]


def single_colorcycle(speed):
    arr = toarray("5db30002ff0000eb000000000000000000")
    arr[7] = speed.bytevalue
    return [arr, SET_MESSAGE, APPLY_MESSAGE]


def multi_static(color1, color2, color3, color4):
    result = []
    for idx, color in enumerate([color1, color2, color3, color4], 1):
        arr = toarray("5db30100ff0000eb000000000000000000")
        arr[2] = idx
        arr[4] = color.red
        arr[5] = color.green
        arr[6] = color.blue
        result.append(arr)
    return result + [SET_MESSAGE, APPLY_MESSAGE]


def multi_breathing(color1, color2, color3, color4, speed):
    result = []
    for idx, color in enumerate([color1, color2, color3, color4], 1):
        arr = toarray("5db30101ff0000eb000000000000000000")
        arr[2] = idx
        arr[4] = color.red
        arr[5] = color.green
        arr[6] = color.blue
        arr[7] = speed.bytevalue
        result.append(arr)
    return result + [SET_MESSAGE, APPLY_MESSAGE]


def rainbow():
    return multi_static(
        Color.fromhexstr("ff0000"),
        Color.fromhexstr("ffff00"),
        Color.fromhexstr("00ffff"),
        Color.fromhexstr("ff00ff"),
    )


def red():
    return single_static(Color.fromhexstr("ff0000"))


def green():
    return single_static(Color.fromhexstr("00ff00"))


def yellow():
    return single_static(Color.fromhexstr("ffff00"))


def blue():
    return single_static(Color.fromhexstr("0000ff"))


def white():
    return single_static(Color.fromhexstr("ffffff"))


COLOR_FUNCS = [
    single_static,
    single_breathing,
    single_colorcycle,
    multi_static,
    multi_breathing,
    rainbow,
    red,
    green,
    yellow,
    gold,
    blue,
    white,
]

USAGE = """rogauracore.py -- Asus Rog RGB keyboard control
(c) 2019 Will Roberts

Syntax:
    python3 rogauracore.py MODE [args...]

MODE is one of {}.
"""


def parseargs(argv):
    # first identify the function the user wants
    mapping = dict((x.__name__, x) for x in COLOR_FUNCS)
    name = None
    if argv:
        name = argv[0]
    if (
        not argv
        or "-?" in argv
        or "-h" in argv
        or "--help" in argv
        or name not in mapping
    ):
        print(USAGE.format(", ".join([x.__name__ for x in COLOR_FUNCS])))
        if name and name not in mapping:
            print("Unknown MODE {}".format(name))
        sys.exit(1)
    func = mapping[name]
    # then figure out the arguments to that function
    argnames = inspect.signature(func).parameters.keys()
    if len(argv[1:]) != len(argnames):
        print("Arguments for MODE {}: {}".format(name, ", ".join(argnames)))
        print("\ncolor arguments should be given as hex values like ff0000 or #ff0000")
        print("speed argument should be given as an integer: 1, 2, or 3")
        sys.exit(1)
    # we know that the arguments match up with the function signature,
    # we just need to parse the values now
    args = {}
    for argname, value in zip(argnames, argv[1:]):
        if argname.startswith("color"):
            args[argname] = Color.fromhexstr(value)
        elif argname == "speed":
            args[argname] = Speed(int(value))
        else:
            raise Exception(
                "Undecipherable argument name {} in function {}".format(argname, name)
            )
    # now we have the args, we use them to invoke the proper function
    return func(**args)


def main():
    try:
        messages = parseargs(sys.argv[1:])
    except Exception as e:
        print(e)
        sys.exit(1)

    dev, iface, active_iface = acquire_usb()

    for message in messages:
        bmRequestType = 0x21
        bRequest = 0x09
        wValue = 0x035D
        wIndex = 0

        try:
            dev.ctrl_transfer(bmRequestType, bRequest, wValue, wIndex, message)
        except usb.USBError as e:
            release_usb(dev, iface, active_iface)
            raise e

    release_usb(dev, iface, active_iface)


if __name__ == "__main__":
    main()
