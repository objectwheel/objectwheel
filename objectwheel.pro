CONFIG      += ordered
TEMPLATE     = subdirs

OTHER_FILES += $$PWD/TODO

SUBDIRS      = $$PWD/utils/utils.pro \
               $$PWD/interpreter/interpreter.pro \
               $$PWD/renderer/renderer.pro \
               $$PWD/themer/themer.pro \
               $$PWD/objectwheel/objectwheel.pro
