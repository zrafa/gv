#!/usr/bin/python
# -*- coding: utf-8 -*-

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

from PelosVisionControl import PelosVisionControl
from PelosConfig import PelosConfig
from IpVideoCapture import IpVideoCapture

VERSION = " v00.11"


class StatusBar(Frame):
    def __init__(self, parent):
        Frame.__init__(self, parent)
        self.label = Label(self, relief=SUNKEN)
        self.label.pack(fill=X, expand=1, side=LEFT)

    def set(self, format, *args):
        self.label.config(text=format % args)
        self.label.update_idletasks()

    def clear(self):
        self.label.config(text="")
        self.label.update_idletasks()

class PelosVisionGUI(Frame):
    def __init__(self, parent, cam, ipcam, control):
        Frame.__init__(self, parent)
   
        self.parent = parent
        self.cam = cam
        self.ipcam = ipcam
        self.control = control

        self.output_path = "./fotos"

        self.parent.title("Análisis Digital de Diámetro de Fibra UNCOMA" + VERSION)

        # crea barra de menu
        menubar = Menu(parent)

        # create a pulldown menu, and add it to the menu bar
        self.filemenu = Menu(menubar, tearoff=0)
        self.filemenu.add_command(label="Abrir", command=self.tomarFotoDeArchivo)
        self.filemenu.add_separator()
        self.filemenu.add_command(label="Salir", command=parent.quit)
        menubar.add_cascade(label="Archivo", menu=self.filemenu)

        self.editmenu = Menu(menubar, tearoff=0)
        self.editmenu.add_command(label="Preferencias", command=self.cambiarPreferencias)
        menubar.add_cascade(label="Editar", menu=self.editmenu)

        helpmenu = Menu(menubar, tearoff=0)
        helpmenu.add_command(label="Acerca de ...", command=self.acercade)
        menubar.add_cascade(label="Ayuda", menu=helpmenu)

        # muestra barra de menu 
        parent.config(menu=menubar)

        # Crea contenedor central
        framecentral = Frame(self.parent)
        framecentral.pack()

        # Panel estadisticas
        lbl = Label(framecentral, text="Estadistica obtenida :")
        lbl.grid(row=0,column=0, sticky=W, pady=1, padx=5)
        self.lbl_estadistica = Text(framecentral, height=10, width=60, state='disabled')
        self.lbl_estadistica.grid(row=0, column=1, sticky=W, pady=1, padx=5) 
        self.lbl_estadistica.insert(END, "")

        # Medida del diámetro en pixels
        lbl = Label(framecentral, text="Diámetro obtenido (MEDIA, en pixels)")
        lbl.grid(row=1,column=0, sticky=W, pady=1, padx=5) 
        self.grosor = Entry(framecentral, state='readonly')

        self.grosor.grid(row=1, column=1, columnspan=1, padx=1, sticky=E+W+S+N)
        
        # Medida del diámetro en micrones
        lbl = Label(framecentral, text="Diametro obtenido (MEDIA, en micrones)")
        lbl.grid(row=2,column=0, sticky=W, pady=1, padx=5) 
        self.grosormicron = Entry(framecentral, state='readonly')
        self.grosormicron.grid(row=2, column=1, columnspan=1, padx=1, sticky=E+W+S+N)

        # Captura imagen      
        lbl = Label(framecentral, text="Foto")
        lbl.grid(row=3,column=0, sticky=W, pady=4, padx=5)
        foto1frame = Frame(framecentral, width=400, height=300)
        foto1frame.grid(row=4, column=0, sticky=E+W+S+N)

        self.ratiobtnframe = Frame(foto1frame)
        self.tomarfotobtn = Button(self.ratiobtnframe, text="Tomar Foto", command=self.tomarFoto, state="disabled")
        self.tomarfotobtn.pack(side=LEFT)

        self.vssrc = IntVar()
        localratiobtn = Radiobutton(self.ratiobtnframe, text="Local Cam", variable=self.vssrc, value=1, command=self.vsconf)
        localratiobtn.pack(side=LEFT)
        remoteratiobtn = Radiobutton(self.ratiobtnframe, text="Remote Cam", variable=self.vssrc, value=2, command=self.vsconf)
        remoteratiobtn.pack(side=LEFT)
        archivoratiobtn = Radiobutton(self.ratiobtnframe, text="Archivo", variable=self.vssrc, value=3, command=self.vsconf)
        archivoratiobtn.select()
        archivoratiobtn.pack(side=LEFT)
        self.ratiobtnframe.pack()
        self.foto1label = Label(foto1frame)
        self.foto1label.pack()

        lbl = Label(framecentral, text="Representación del Análisis")
        lbl.grid(row=3,column=1, sticky=W, pady=4, padx=5)
        foto2frame = Frame(framecentral, width=400, height=300)
        foto2frame.grid(row=4, column=1, sticky=W+S)
        self.foto2label = Label(foto2frame)
        self.foto2label.pack(fill=X)

        # crea y muestra barra de estado
        self.status = StatusBar(parent)
        self.status.pack(fill=X)

        self.preferencias = False
        
        self.vsconf()

        self.video_loop()

    def video_loop(self):
        imgsrc = self.vssrc.get()
        if imgsrc != 3: 
            if imgsrc == 2:
                ok, self.cv2image = self.ipcam.read()
                self.status.set("Camara remota está lista.")

            elif imgsrc == 1:
                ok, self.cv2image = self.cam.read()  # read frame from video stream

            if ok:  # frame captured without any errors
                self.cv2image = cv2.cvtColor(self.cv2image, cv2.COLOR_BGR2GRAY)
                self.mostrarImagen(self.foto1label, self.cv2image)
                
        self.master.after(30, self.video_loop)  # call the same function after 30 milliseconds

    def vsconf(self):
        if self.vssrc.get() == 3:
            self.tomarfotobtn.configure(state='disabled')
            self.filemenu.entryconfig("Abrir", state='normal')

            self.cam.release()

            if self.ipcam.isOpened():
                self.ipcam.close()

            self.status.set("Analisis de archivo listo.")
        else:
            self.tomarfotobtn.configure(state='normal')
            self.filemenu.entryconfig("Abrir", state='disabled')

            if self.vssrc.get() == 1:
                self.setcam()
            elif self.vssrc.get() == 2:
                self.cam.release()

                self.ipcam.open(int(self.control.config.port))
                if self.ipcam.isOpened():
                    self.status.set("Camara remota lista. Port %d.", int(self.control.config.port))
                else:
                    self.status.set("Camara remota no está lista.")
    
    def mostrarImagen(self, label, cv2image):
        img = cv2.resize(cv2image, (400, 300))
        img = Image.fromarray(img)
        img = ImageTk.PhotoImage(img)
        label.configure(image=img)
        label.image = img

    def tomarFotoDeArchivo(self):
        myfiletypes = [('Python files', '*.pgm')]
        open = tkFileDialog.Open(self.parent, filetypes = myfiletypes)
        self.filename = open.show()
        if self.filename != '':
            self.cv2image = cv2.imread(self.filename)
            self.mostrarImagen(self.foto1label, self.cv2image)
            self.analizarFoto()

    def tomarFoto(self):
        ts = datetime.datetime.now() # grab the current timestamp
        self.filename = "{}.pgm".format(ts.strftime("%Y-%m-%d_%H-%M-%S"))  # construct filename
        self.filename = os.path.join(self.output_path, self.filename)  # construct output path
        cv2.imwrite(self.filename, self.cv2image)
        
        self.analizarFoto()

    def analizarFoto(self):
        self.status.set("Analizando %s.", self.filename)
        
        self.grosorpx = float(self.control.calcularGrosorPx(self.filename))
        self.grosormu = self.grosorpx*float(self.control.config.micron)

        self.clasicas = self.control.calcularClasicas(self.filename)
        self.rangos = self.control.calcularRangos(self.filename)
        
        self.mostrarResultado()

        self.status.set("")

    def mostrarResultado(self):
        # Hay que ejecutar para hacer merge de los EPS generados :
        # convert salida.ps grosordelpelo.eps -layers merge salida2.png
        output = Popen(["convert", "salida.eps", "grosordelpelo.eps", "-layers", "merge", "salida2.png"], stdout=PIPE).communicate()[0]

        self.cv2image2 = cv2.imread("salida2.png")
        self.mostrarImagen(self.foto2label, self.cv2image2)

        # grosor en pixels
        self.grosor.configure(state='normal')
        self.grosor.delete(0, END)
        self.grosor.insert(0, self.grosorpx)
        self.grosor.configure(state='readonly')

        # grosor en micrones
        self.grosormicron.configure(state='normal')
        self.grosormicron.delete(0, END)
        self.grosormicron.insert(0, self.grosormu)
        self.grosormicron.configure(state='readonly')

        self.lbl_estadistica.configure(state='normal')
        self.lbl_estadistica.delete('1.0', END)
        # Las mediciones por rangos
        self.lbl_estadistica.insert('1.0', self.rangos)
        # Las mediciones clasicas
        self.lbl_estadistica.insert('4.0', '\n')
        self.lbl_estadistica.insert('5.0', self.clasicas)
        self.lbl_estadistica.configure(state='disabled')

    def cambiarPreferencias(self):
        if (self.preferencias == False):
            self.preferencias = True
            self.editmenu.entryconfig("Preferencias", state="disabled")
            self.preferenciastop = Toplevel()
            self.preferenciastop.title("Preferencias")
            self.preferenciastop.protocol('WM_DELETE_WINDOW', self.cerrarPreferencias)

            prefframe = Frame(self.preferenciastop)
            prefframe.grid(row=0, column=0, sticky=W)

            lbl = Label(prefframe, text="Número de la Video Camara")
            lbl.grid(row=0,column=0, sticky=W, pady=1, padx=5)
            self.camara = Entry(prefframe)
            self.camara.grid(row=0, column=1, columnspan=1, padx=1, sticky=E+W+S+N)
            self.camara.delete(0, END)
            self.camara.insert(0, self.control.config.camara)

            lbl = Label(prefframe, text="UDP port")
            lbl.grid(row=1,column=0, sticky=W, pady=1, padx=5)
            self.port = Entry(prefframe)
            self.port.grid(row=1, column=1, columnspan=1, padx=1, sticky=E+W+S+N)
            self.port.delete(0, END)
            self.port.insert(0, self.control.config.port)

            lbl = Label(prefframe, text="Calibracion 1 pixel = (micrones)")
            lbl.grid(row=2,column=0, sticky=W, pady=1, padx=5) 
            self.micron = Entry(prefframe)
            self.micron.grid(row=2, column=1, columnspan=1, padx=1, sticky=E+W+S+N)
            self.micron.delete(0, END)
            self.micron.insert(0, self.control.config.micron)

            lbl = Label(prefframe, text="Valor minimo de aceptacion de diametro (en pixels)")
            lbl.grid(row=3,column=0, sticky=W, pady=1, padx=5) 
            self.minimo = Entry(prefframe)
            self.minimo.grid(row=3, column=1, columnspan=1, padx=1, sticky=E+W+S+N)
            self.minimo.delete(0, END)
            self.minimo.insert(0, self.control.config.minimo)
      
            lbl = Label(prefframe, text="Valor máximo de aceptación de diametro (en pixels)")
            lbl.grid(row=4,column=0, sticky=W, pady=4, padx=5) 
            self.limite = Entry(prefframe)
            self.limite.grid(row=4, column=1, columnspan=1, padx=1, sticky=E+W+S+N)
            self.limite.delete(0, END)
            self.limite.insert(0, self.control.config.limite)

    def setres(self, cap, x,y):
        cap.set(cv2.cv.CV_CAP_PROP_FRAME_WIDTH, int(x))
        cap.set(cv2.cv.CV_CAP_PROP_FRAME_HEIGHT, int(y))
        return str(cap.get(cv2.cv.CV_CAP_PROP_FRAME_WIDTH)),str(cap.get(cv2.cv.CV_CAP_PROP_FRAME_HEIGHT))

    def setcam(self):
        self.cam.release()
        self.cam.open(int(self.control.config.camara))
        if self.cam.isOpened():
            res = self.setres(self.cam, 1280, 960)
            self.status.set("Camara %d lista %s.", int(self.control.config.camara), res)
        else:
            self.status.set("Camara %d no está lista.", int(self.control.config.camara))

    def cerrarPreferencias(self):
        self.control.config.camara = self.camara.get()

        self.setcam()
        
        self.control.config.port = self.port.get()

        self.control.config.micron = self.micron.get()
        self.control.config.minimo = self.minimo.get()
        self.control.config.limite = self.limite.get()

        self.control.config.guardar()

        self.preferencias = False
        self.editmenu.entryconfig("Preferencias", state="normal")
        self.preferenciastop.destroy()

    def acercade(self):
        label = tkMessageBox.showinfo("Acerca de ...", "Analisis Digital de Diametro de Fibra UNCOMA v00.11\n\nEste programa en Python Tk y Opencv toma una foto desde una camara usb, la guarda en formato PGM, la analiza con lsd y calcula el diametro (media) de fibras en la foto. Presenta la foto capturada y sus resultados. \n\nCopyright (C) 2015, 2017 Rafael Ignacio Zurita y Rodolfo del Castillo\n\nFacultad de Informatica\nUniversidad Nacional del Comahue\n\nThis program is free software; you can redistribute it and/or modify it under the terms of the GPL v2")

  
def main():
    root = Tk()

    config = PelosConfig('pelos.cfg')
    control = PelosVisionControl(config)
    cam = cv2.VideoCapture()
    ipcam = IpVideoCapture(root)

    PelosVisionGUI(root, cam, ipcam, control)

    root.mainloop()

if __name__ == '__main__':
    main()
