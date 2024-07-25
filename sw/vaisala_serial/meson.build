project('vaisala_serial', 'c')

incdir = include_directories('../../fw/chaac/src/lib/vaisala/')
src = ['../../fw/chaac/src/lib/vaisala/vaisala_crc.c', '../../fw/chaac/src/lib/vaisala/vaisala.c', 'main.c']

executable('vaisala_serial', sources : src, include_directories: incdir)