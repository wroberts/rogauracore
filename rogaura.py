#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import array
import binascii
import sys
import usb.core
import usb.util

ASUS_VENDOR_ID = 0x0b05
ASUS_PRODUCT_ID = 0x1869

def acquire():
    dev = usb.core.find(idVendor=ASUS_VENDOR_ID, idProduct=ASUS_PRODUCT_ID)
    if dev is None:
        print('Could not find ASUS RGB keyboard.')
        sys.exit(1)

    #print(dev)

    try:
        # get the keyboard USB interface
        assert dev.bNumConfigurations == 1
        cfg = dev.configurations()[0]
        assert cfg.bNumInterfaces == 1
        iface = 0

        #cfg = dev.get_active_configuration()
        #iface = cfg[(0,0)]
        active_iface = False
        if dev.is_kernel_driver_active(iface):
            dev.detach_kernel_driver(iface)
            active_iface = True
    except usb.USBError as e:
        usb.util.dispose_resources(dev)
        raise e

    # set the active configuration. With no arguments, the first
    # configuration will be the active one
    #dev.set_configuration()

    try:
        usb.util.claim_interface(dev, iface)
    except usb.USBError as e:
        if active_iface:
            dev.attach_kernel_driver(iface)
        usb.util.dispose_resources(dev)
        raise e

    return dev, iface, active_iface

def release(dev, iface, active_iface):
    usb.util.release_interface(dev, iface)
    if active_iface:
        dev.attach_kernel_driver(iface)
    usb.util.dispose_resources(dev)

def single_static(color):
    pass

def single_breathing(color1, color2, speed):
    pass

def single_colorcycle(speed):
    pass

def multi_static(color1, color2, color3, color4):
    pass

def multi_breathing(color1, color2, color3, color4, speed):
    pass

CONVOS = [
    ('static rainbow',
     ['5db30100ff0000eb000000000000000000',
      '5db30200ffff00eb000000000000000000',
      '5db3030000ffffeb000000000000000000',
      '5db30400ff00ffeb000000000000000000',
      '5db5000000000000000000000000000000']),
    #('',
    # ['5db4000000000000000000000000000000',
    #  '5db5000000000000000000000000000000']),
    ('static red',
     [#'5d4153555320546563682e496e632e0000  ]ASUS Tech.Inc.',
      #'5d05203100080000000000000000000000',
      '5db30000ff000000000000000000000000',
      '5db5000000000000000000000000000000']),
    ('rainbow cycle med',
     ['5db30002ff0000eb000000000000000000',
      '5db5000000000000000000000000000000']),
    ('rainbow cycle slow',
     ['5db30002ff0000e1000000000000000000',
      '5db5000000000000000000000000000000']),
    ('rainbow cycle med',
     ['5db30002ff0000eb000000000000000000',
      '5db5000000000000000000000000000000']),
    ('rainbow cycle fast',
     ['5db30002ff0000f5000000000000000000',
      '5db5000000000000000000000000000000']),
    ('breathe med',
     ['5db3000108fff0eb0001ff000000000000',
      '5db5000000000000000000000000000000']),
    ('breathe slow',
     ['5db3000108fff0e10001ff000000000000',
      '5db5000000000000000000000000000000']),
    ('breathe med',
     ['5db3000108fff0eb0001ff000000000000',
      '5db5000000000000000000000000000000']),
    ('breathe fast',
     ['5db3000108fff0f50001ff000000000000',
      '5db5000000000000000000000000000000']),
    ('static red',
     ['5db30000ff000000000000000000000000',
      '5db5000000000000000000000000000000']),
    ('static rainbow',
     ['5db30100ff0000eb000000000000000000',
      '5db30200ffff00eb000000000000000000',
      '5db3030000ffffeb000000000000000000',
      '5db30400ff00ffeb000000000000000000',
      '5db5000000000000000000000000000000']),
    ('rainbow breathe med',
     ['5db30101ff0000eb000000000000000000',
      '5db30201ffff00eb000000000000000000',
      '5db3030100ffffeb000000000000000000',
      '5db30401ff00ffeb000000000000000000',
      '5db5000000000000000000000000000000']),
    ('rainbow breathe slow',
     ['5db30101ff0000e1000000000000000000',
      '5db30201ffff00e1000000000000000000',
      '5db3030100ffffe1000000000000000000',
      '5db30401ff00ffe1000000000000000000',
      '5db5000000000000000000000000000000']),
    ('rainbow breathe med',
     ['5db30101ff0000eb000000000000000000',
      '5db30201ffff00eb000000000000000000',
      '5db3030100ffffeb000000000000000000',
      '5db30401ff00ffeb000000000000000000',
      '5db5000000000000000000000000000000']),
    ('rainbow breathe fast',
     ['5db30101ff0000f5000000000000000000',
      '5db30201ffff00f5000000000000000000',
      '5db3030100fffff5000000000000000000',
      '5db30401ff00fff5000000000000000000',
      '5db5000000000000000000000000000000']),
    ('rainbow breathe slow',
     ['5db30101ff0000e1000000000000000000',
      '5db30201ffff00e1000000000000000000',
      '5db3030100ffffe1000000000000000000',
      '5db30401ff00ffe1000000000000000000',
      '5db5000000000000000000000000000000']),
    ('static rainbow',
     ['5db30100ff0000eb000000000000000000',
      '5db30200ffff00eb000000000000000000',
      '5db3030000ffffeb000000000000000000',
      '5db30400ff00ffeb000000000000000000',
      '5db5000000000000000000000000000000']),
    #('',
    # ['5db4000000000000000000000000000000',
    #  '5db5000000000000000000000000000000']),
    ('static rainbow',
     ['5db30100ff0000eb000000000000000000',
      '5db30200ffff00eb000000000000000000',
      '5db3030000ffffeb000000000000000000',
      '5db30400ff00ffeb000000000000000000',
      '5db5000000000000000000000000000000']),
    #('',
    # ['5db4000000000000000000000000000000',
    #  '5db5000000000000000000000000000000']),
]

def main():
    # replay recorded conversations
    last_convo_name = None
    untested_convo = None
    for convo_name, convo in CONVOS:
        if convo_name == '':
            print('found untested convo.  last tested convo was {}'.format(last_convo_name))
            untested_convo = convo
            break
        last_convo_name = convo_name
    if untested_convo is None:
        print('no untested convo found')
        sys.exit(1)

    dev, iface, active_iface = acquire()

    for speech in untested_convo:
        msg = array.array('B')
        #msg.frombytes(binascii.unhexlify(b'5db30101ff0000eb000000000000000000'))
        msg.frombytes(binascii.unhexlify(speech))
        # msg
        bmRequestType = 0x21
        bRequest = 0x09
        wValue = 0x035d
        wIndex = 0

        print('Sending: {}'.format(speech))
        try:
            dev.ctrl_transfer(bmRequestType, bRequest, wValue, wIndex, msg)
        except usb.USBError as e:
            release(dev, iface, active_iface)
            raise e

    release(dev, iface, active_iface)

if __name__ == '__main__':
    main()

sys.exit(0)

# ret = dev.ctrl_transfer(0xC0, CTRL_LOOPBACK_READ, 0, 0, len(msg))

# # get an endpoint instance
# cfg = dev.get_active_configuration()
# iface = cfg[(0,0)]

# ep = usb.util.find_descriptor(
#     iface,
#     # match the first IN endpoint
#     custom_match = \
#     lambda e: \
#         usb.util.endpoint_direction(e.bEndpointAddress) == \
#         usb.util.ENDPOINT_IN)

# assert ep is not None

# # write the data
# ep.write('test')



# all control transfer packets defined in
# https://github.com/MidhunSureshR/openauranb/blob/master/rog_aura.c
# start with bmRequestType 0x21
#
# bin(0x21) => 00100001
# 0: host-to-device
# 01: class
# 00001: interface
#
# then we have bRequest as 0x09
# bin(0x09) =? 00001001 (9) SET_CONFIGURATION
#
# then we have wValue as 0x035D (861), wIndex is 0x0000, and wLength is 17
#
# packet payloads always start with 0x5d
#
# bin(0x5d) => 01011101
#
# 0: host-to-device
# 10: vendor
# 11101: reserved? (29)
#
# after that, we've got variously 0xb3 (color value), b4 (apply), or b5 (set)
# bin(0xb4) = 10110100 = 180
# bin(0xb5) = 10110101 = 181
# bin(0xb6) = 10110110 = 182
#
# for 0xb4 and b5, after that we've got all zeroes
#
# we've always got 17 bytes in each message
#
# lsusb gives:
# wMaxPacketSize     0x0040  1x 64 bytes
# bInterval               4
#
# in the case of 0xb3, we follow with 0x0000, and then the color data
# comes (3 bytes); after the 0x0000, the message length has room for
# 13 bytes (4 x RGB + 1 byte mode?).  maybe the 0x0000 has mode
# information?

MESSAGES = [
    '5db30101ff0000eb000000000000000000',
    '5db30201ffff00eb000000000000000000',
    '5db3030100ffffeb000000000000000000',
    '5db30401ff00ffeb000000000000000000',
    '5db5000000000000000000000000000000',
    '5db30100ff0000eb000000000000000000',
    '5db30200ffff00eb000000000000000000',
    '5db3030000ffffeb000000000000000000',
    '5db30400ff00ffeb000000000000000000',
    '5db5000000000000000000000000000000',
    '5db4000000000000000000000000000000',
    '5db5000000000000000000000000000000',
    '5d4153555320546563682e496e632e0000  ]ASUS Tech.Inc.',
    '5d05203100080000000000000000000000',
    '5db30000ff000000000000000000000000',
    '5db5000000000000000000000000000000',
    '5db30002ff0000eb000000000000000000',
    '5db5000000000000000000000000000000',
    '5db30002ff0000e1000000000000000000',
    '5db5000000000000000000000000000000',
    '5db30002ff0000eb000000000000000000',
    '5db5000000000000000000000000000000',
    '5db30002ff0000f5000000000000000000',
    '5db5000000000000000000000000000000',
    '5db3000108fff0eb0001ff000000000000',
    '5db5000000000000000000000000000000',
    '5db3000108fff0e10001ff000000000000',
    '5db5000000000000000000000000000000',
    '5db3000108fff0eb0001ff000000000000',
    '5db5000000000000000000000000000000',
    '5db3000108fff0f50001ff000000000000',
    '5db5000000000000000000000000000000',
    '5db30000ff000000000000000000000000',
    '5db5000000000000000000000000000000',
    '5db30100ff0000eb000000000000000000',
    '5db30200ffff00eb000000000000000000',
    '5db3030000ffffeb000000000000000000',
    '5db30400ff00ffeb000000000000000000',
    '5db5000000000000000000000000000000',
    '5db30101ff0000eb000000000000000000',
    '5db30201ffff00eb000000000000000000',
    '5db3030100ffffeb000000000000000000',
    '5db30401ff00ffeb000000000000000000',
    '5db5000000000000000000000000000000',
    '5db30101ff0000e1000000000000000000',
    '5db30201ffff00e1000000000000000000',
    '5db3030100ffffe1000000000000000000',
    '5db30401ff00ffe1000000000000000000',
    '5db5000000000000000000000000000000',
    '5db30101ff0000eb000000000000000000',
    '5db30201ffff00eb000000000000000000',
    '5db3030100ffffeb000000000000000000',
    '5db30401ff00ffeb000000000000000000',
    '5db5000000000000000000000000000000',
    '5db30101ff0000f5000000000000000000',
    '5db30201ffff00f5000000000000000000',
    '5db3030100fffff5000000000000000000',
    '5db30401ff00fff5000000000000000000',
    '5db5000000000000000000000000000000',
    '5db30101ff0000e1000000000000000000',
    '5db30201ffff00e1000000000000000000',
    '5db3030100ffffe1000000000000000000',
    '5db30401ff00ffe1000000000000000000',
    '5db5000000000000000000000000000000',
    '5db30100ff0000eb000000000000000000',
    '5db30200ffff00eb000000000000000000',
    '5db3030000ffffeb000000000000000000',
    '5db30400ff00ffeb000000000000000000',
    '5db5000000000000000000000000000000',
    '5db4000000000000000000000000000000',
    '5db5000000000000000000000000000000',
    '5db30100ff0000eb000000000000000000',
    '5db30200ffff00eb000000000000000000',
    '5db3030000ffffeb000000000000000000',
    '5db30400ff00ffeb000000000000000000',
    '5db5000000000000000000000000000000',
    '5db4000000000000000000000000000000',
    '5db5000000000000000000000000000000',
]

cs = []
c = []
for m in MESSAGES:
    c.append(m)
    if m == '5db5000000000000000000000000000000':
        c = []
        cs.append(c)
