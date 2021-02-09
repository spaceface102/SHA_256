#!/usr/bin/python3
from sys import argv
import os
if len(argv) == 1:
    print("Yeah, something went really wrong huh...")
else:
    if os.path.exists(argv[1]):
          os.remove(argv[1])
