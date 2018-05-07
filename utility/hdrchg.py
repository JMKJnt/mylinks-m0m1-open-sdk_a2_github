#!/usr/bin/python2.7

import sys
import os
import struct

src = open(sys.argv[1], "rb")
dst = open(sys.argv[2], "wb")
buf = src.read(12)

dst.write(buf)
#read romver
rom_ver = struct.unpack("!H",  src.read(2))
a = rom_ver[0] + 1
dst.write(struct.pack("!H", a))

buf = src.read(14)
dst.write(buf)

#read checksum
chksum = struct.unpack("!H",  src.read(2))
b = chksum[0] - 1
dst.write(struct.pack("!H", b))

buf = src.read()
dst.write(buf)

	
src.close()
dst.close()
