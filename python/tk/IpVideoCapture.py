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

PACK_SIZE = 4096

class IpVideoCapture():
    def __init__(self, master, port=8000):

        self.master = master

        self.port = port

#        self.img = np.zeros([640, 480, 3], dtype=np.uint8)
        self.img = cv2.imread('fai.jpg')

        self.imglista = True
        self.isopen = False

        self.total_pack = 0
        self.data = bytearray()
       
    def ihandler(self, sock, mask):
        buf = bytearray(PACK_SIZE)
        nbytes, sender = sock.recvfrom_into(buf, PACK_SIZE)
        self.data.extend(buf[:nbytes])

        if nbytes == 4:
            self.total_pack = socket.ntohl(struct.unpack('I', self.data[:nbytes])[0])
            self.data = bytearray()       
        else:
            self.total_pack -= 1
            if self.total_pack == 0:
                cv2image = np.asarray(self.data, np.uint8)
                self.img = cv2.imdecode(cv2image, cv2.CV_LOAD_IMAGE_COLOR)
                self.imglista = True

                self.data = bytearray()
            else:
                self.imglista = False
                
    def open(self, port):
        self.port = port

        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socket.bind(('', self.port))

        self.master.createfilehandler(self.socket, READABLE, self.ihandler)

        self.isopen = True

    def read(self):
        return (self.imglista, self.img)

    def isOpened(self):
        return self.isopen

    def close(self):
        self.master.deletefilehandler(self.socket)
        self.socket.close()
        self.isopen = False

