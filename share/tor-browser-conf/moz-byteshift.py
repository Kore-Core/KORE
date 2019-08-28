#!/usr/bin/python3

import sys

def bytes_shift(input_file, output_file, bytes_count = 13):
  shifted_data = []

  with open(input_file, "rb") as input_file_buffer:
    input_data = input_file_buffer.read()
    unshifted_data = list(input_data)

    for byte_number in unshifted_data:
      byte_number_shifted = byte_number + bytes_count
      if byte_number_shifted > 256:
        byte_number_shifted = byte_number_shifted - 256
      shifted_data.append(byte_number_shifted)

  output_data = bytes(shifted_data)

  with open(output_file, "wb") as output_file_buffer:
    output_file_buffer.write(output_data)

if __name__ == '__main__':
  input_file = None
  output_file = None
  bytes_count = 13

  for arg in sys.argv:
    if arg.startswith('-i='):
      input_file = arg[3:]
    elif arg.startswith('-o='):
      output_file = arg[3:]
    elif arg.startswith('-n='):
      bytes_count = int(arg[3:])

  bytes_shift(input_file, output_file, bytes_count)