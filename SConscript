import os
from building import *
import rtconfig

cwd  = GetCurrentDir()

src  = []
CPPPATH = []
CPPDEFINES = []
LOCAL_CCFLAGS = ''

if GetDepend(['PKG_USING_TOUCH']):
    src += Glob('src/touch.c')
    
if GetDepend(['FT3x07']):
    src += Glob('src/device/ft3x07.c')

CPPPATH += [cwd]

group = DefineGroup('Touch', src, depend = ['PKG_USING_TOUCH'], CPPPATH = CPPPATH, LOCAL_CCFLAGS = LOCAL_CCFLAGS, CPPDEFINES = CPPDEFINES)
Return('group')