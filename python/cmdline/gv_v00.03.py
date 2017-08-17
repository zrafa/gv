#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
from subprocess import Popen, PIPE, STDOUT

from PelosConfig import PelosConfig
from PelosVisionControl import PelosVisionControl

def main():
    if len(sys.argv) == 2:
        config = PelosConfig('pelos.cfg')

        app = PelosVisionControl(config)

        print (app.calcularGrosorPx(sys.argv[1]))

    else:
       print "Ejecutar:"
       print "  python gv_v00.00 nombrearchivo.pgm"

if __name__ == '__main__':
    main()
