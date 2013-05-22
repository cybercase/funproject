import sys
import os

# **********************************************
#                 Descriptors
# **********************************************

class Descriptor:
    def __init__(self, name=None):
        self.name = name
    def __set__(self, instance, value):
        instance.__dict__[self.name] = value
    def __delete__(self, instance):
        raise AttributeError("Can't delete")

class Typed(Descriptor):
    ty = object
    def __set__(self, instance, value):
        if not isinstance(value, self.ty):
            raise TypeError('Expected %s' % self.ty)
        super().__set__(instance, value)

class Sized(Descriptor):
    def __init__(self, *args, maxlen, **kwargs):
        self.maxlen = maxlen
        super().__init__(*args, **kwargs)
    def __set__(self, instance, value):
        if len(value) > self.maxlen:
            raise ValueError('Too big')
        super().__set__(instance, value)

class String(Typed):
    ty = str

class Integer(Typed):
    ty = int

class Float(Typed):
    ty = float

class Positive(Descriptor):
    def __set__(self, instance, value):
        if value < 0:
            raise ValueError('Expected >= 0')
        super().__set__(instance, value)

class PosInteger(Integer, Positive):
    pass

class PosFloat(Float, Positive):
    pass

class SizedString(String, Sized):
	pass

# **********************************************
#                 Metaclass
# **********************************************

from collections import OrderedDict
class StructMeta(type):
    @staticmethod
    def __prepare__(cls, name, bases=None):
        # this method returns the dictionary used by the class instance.
        # To generate a signature the parameters order is crucial, so an
        # OrderedDict is used
        return OrderedDict()

    def __new__(cls, name, bases, clsdict):
        fields = [ key for key, val in clsdict.items()
                if isinstance(val, Descriptor) ]
        for name in fields:
            clsdict[name].name = name
        
        # __init__
        if len(fields):
	        init_code = 'def __init__(self, %s):\n' % \
	                    ', '.join(fields)
	        for name in fields:
	            init_code += '    self.%s = %s\n' % (name, name)
	        exec(init_code, globals(), clsdict)
        
        clsobj = super().__new__(cls, name, bases, dict(clsdict))
        return clsobj

# **********************************************
#                 Base Class
# **********************************************

class Structure(metaclass=StructMeta):
    pass

# **********************************************
#                 XML Parsing
# **********************************************
from xml.etree.ElementTree import parse
def _xml_to_code(filename):
    doc = parse(filename)
    code = ''
    for st in doc.findall('structure'):
        code += _xml_struct_code(st)
    return code

def _xml_struct_code(st):
    stname = st.get('name')
    code = 'class %s(Structure):\n' % stname
    for field in st.findall('field'):
        name = field.text.strip()
        dtype = field.get('type')
        kwargs = ', '.join('%s=%s' % (key, val)
                            for key, val in field.items()
                            if key != 'type')
        code += '    %s = %s(%s)\n' % (name, dtype, kwargs)
    return code

# **********************************************
#                 Module Import
# **********************************************
class StructImporter:
    def __init__(self, path):
        self._path = path
    def find_module(self, fullname, path=None):
        name = fullname.rpartition('.')[-1]
        if path is None:
            path = self._path
        for dn in path:
            filename = os.path.join(dn, name+'.xml')
            if os.path.exists(filename):
                return StructXMLLoader(filename)
        return None

import imp
class StructXMLLoader:
    def __init__(self, filename):
        self._filename = filename
    def load_module(self, fullname):
        mod = sys.modules.setdefault(fullname,
                                    imp.new_module(fullname))
        mod.__file__ = self._filename
        mod.__loader__ = self
        code = _xml_to_code(self._filename)
        # actually this is wrong. I shoul use mod.__dict__ instead of globals,
        # and add an import statement into "code" to load Structure and the other
        # classes
        exec(code, globals(), mod.__dict__)
        return mod

def install_importer(path=sys.path):
    sys.meta_path.append(StructImporter(path))
    
install_importer()