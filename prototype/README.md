# Demo

sudo python3 rogaura.py single_breathing 08fff0 ff0000 1
sudo python3 rogaura.py single_colorcycle 2
sudo python3 rogaura.py multi_breathing ff0000 ffffff ffffff ff0000 2

# Notes

All control transfer packets defined in
https://github.com/MidhunSureshR/openauranb/blob/master/rog_aura.c
start with bmRequestType 0x21

bin(0x21) => 00100001
0: host-to-device
01: class
00001: interface

then we have bRequest as 0x09
bin(0x09) =? 00001001 (9) SET_CONFIGURATION

then we have wValue as 0x035D (861), wIndex is 0x0000, and wLength is 17

packet payloads always start with 0x5d

bin(0x5d) => 01011101

0: host-to-device
10: vendor
11101: reserved? (29)

after that, we've got variously 0xb3 (color value), b4 (apply), or b5 (set)
bin(0xb4) = 10110100 = 180
bin(0xb5) = 10110101 = 181
bin(0xb6) = 10110110 = 182

for 0xb4 and b5, after that we've got all zeroes

we've always got 17 bytes in each message

lsusb gives:
wMaxPacketSize     0x0040  1x 64 bytes
bInterval               4

in the case of 0xb3, we follow with 0x0000, and then the color data
comes (3 bytes); after the 0x0000, the message length has room for
13 bytes (4 x RGB + 1 byte mode?).  maybe the 0x0000 has mode
information?
