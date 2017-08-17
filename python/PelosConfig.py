from ConfigParser import SafeConfigParser


section_names = 'CAMARA', 'CALIBRACION'


class PelosConfig(object):

    def __init__(self, file_name):
        self.file_name = file_name

        self.parser = SafeConfigParser()
        self.parser.optionxform = str  # make option names case sensitive
        found = self.parser.read(file_name)
        if not found:
            raise ValueError('No config file found!')
        for name in section_names:
            self.__dict__.update(self.parser.items(name))  # <-- here the magic happens

    def guardar(self):
        self.parser.set('CAMARA', 'camara', str(self.camara))
        self.parser.set('CAMARA', 'port', str(self.port))

        self.parser.set('CALIBRACION', 'micron', str(self.micron))
        self.parser.set('CALIBRACION', 'minimo', str(self.minimo))
        self.parser.set('CALIBRACION', 'limite', str(self.limite))

        # Guardamos los cambios
        with open(self.file_name, "w") as f:
            self.parser.write(f)
        f.close()


