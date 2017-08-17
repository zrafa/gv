from Tkinter import *
import sys, socket, time, io, os
import cv2
import numpy as np
import struct
from PIL import Image, ImageTk

ECHO_PORT = 8000
PACK_SIZE = 4096

class StatusBar(Frame):

    def __init__(self, master):
        Frame.__init__(self, master)
        self.label = Label(self, bd=1, relief=SUNKEN, anchor=W)
        self.label.pack(fill=X)

    def set(self, format, *args):
        self.label.config(text=format % args)
        self.label.update_idletasks()

    def clear(self):
        self.label.config(text="")
        self.label.update_idletasks()

class GUIServer:
    def __init__(self, master=None):
        self.master = master
        self.master.title('UDP-image-streming-server')
        self.frame = Frame(master, relief=RAISED, borderwidth=2)
        self.text = Text(self.frame, height=26, width=50)
        self.scroll = Scrollbar(self.frame, command=self.text.yview)
        self.text.configure(yscrollcommand=self.scroll.set)
        self.text.pack(side=LEFT)
        self.scroll.pack(side=RIGHT, fill=Y)
        self.frame.pack(padx=4, pady=4)

        self.status = StatusBar(master)
        self.status.pack(side=BOTTOM, fill=X)

        Button(master, text='Close', command=self.master.quit).pack(side=TOP)

        self.total_pack = 0
        self.data = bytearray()       
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socket.bind(('', ECHO_PORT))

        self.master.createfilehandler(self.socket, READABLE, self.ihandler)

        self.master.after(5000, self.doMark)

    def ihandler(self, sock, mask):
        buf = bytearray(PACK_SIZE)
        nbytes, sender = sock.recvfrom_into(buf, PACK_SIZE)
        self.status.set("receiving from: %s %d bytes", sender, nbytes)
        self.data.extend(buf)

        if nbytes == 4:
            self.total_pack = socket.ntohl(struct.unpack('I', self.data[:nbytes])[0])
            self.data = bytearray()       
            print "nbytes: ", nbytes, "total_pack: ", self.total_pack
        else:
            self.total_pack -= 1
            if self.total_pack == 0:
                print "nbytes: ", nbytes, "total_pack: ", self.total_pack
                cv2image = np.asarray(self.data, np.uint8)
                img = cv2.imdecode(cv2image, cv2.CV_LOAD_IMAGE_COLOR)

                cv2.imwrite('out.jpg', img)
                
                self.master.quit()

    def doMark(self):
        self.status.set("%s", 'waiting...')
        self.master.after(5000, self.doMark)

root = Tk()
example = GUIServer(root)
root.mainloop() 
