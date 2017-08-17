from Tkinter import *
import tkFileDialog
import cv2
from PIL import Image, ImageTk
import tkMessageBox

import sys, socket, time, io, os
from subprocess import Popen, PIPE, STDOUT
import numpy as np
import struct
import datetime

class PelosVisionControl():
    def __init__(self, config):       
        self.config = config
        
    def calcularGrosorPx(self, filename):
        output = Popen(["../../gv/gv", "-T", self.config.minimo, "-t", self.config.limite, "-P", "salida.eps", filename, "salida.txt"], stdout=PIPE).communicate()[0]
        output = output.replace('Grosor del PELO en pixels : ', '')
        output = output[:(len(output)-1)]
   
        return output

    def calcularClasicas(self, filename):
        # Las mediciones clasicas
        output = Popen(["./procesar.sh", self.config.minimo, self.config.limite, filename], stdout=PIPE).communicate()[0]

        return output

    def calcularRangos(self, filename):
        # Las mediciones por rangos
        output = Popen(["./procesar_rangos.sh", filename ], stdout=PIPE).communicate()[0]

        return output


