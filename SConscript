from building import *
Import('rtconfig')

src   = []
cwd   = GetCurrentDir()

# add bma400 src files.
src += Glob('sensor_bosch_bma400.c')
src += Glob('libraries/bma400.c')

# add bma400 include path.
path  = [cwd, cwd + '/libraries']

# add src and include to group.
group = DefineGroup('bma400', src, depend = ['PKG_USING_BMA400'], CPPPATH = path)

Return('group')
