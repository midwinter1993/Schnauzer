#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
sys.path.append(os.path.dirname(os.path.realpath(__file__)))
from MYCC import main


if __name__ == "__main__":
    CC = 'clang++'
    WRAP_CC = 'MYCXX'
    main(WRAP_CC, CC)
