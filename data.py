#!/usr/bin/env python3

import argparse
import random
import struct
import sys

ENDIAN_CONV = {
  'n': '=', # native
  'b': '>', # big
  'l': '<', # little
}

def main(args):
  format = '{}Q'.format(ENDIAN_CONV[args.endianness])
  for i in range(args.count):
    value = random.randint(args.range_from, args.range_to)
    args.output.write(struct.pack(format, value))
  args.output.close()


if __name__ == '__main__':
  parser = argparse.ArgumentParser()
  parser.add_argument('--output',     '-o', type=argparse.FileType(mode='wb'))
  parser.add_argument('--range-from', '-l', type=int)
  parser.add_argument('--range-to',   '-u', type=int)
  parser.add_argument('--count',      '-n', type=int)
  parser.add_argument('--endianness', '-e', choices=('n', 'l', 'b'), default='=')
  sys.exit(main(parser.parse_args()) or 0)
