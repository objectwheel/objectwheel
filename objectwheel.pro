CONFIG      += ordered
TEMPLATE     = subdirs

OTHER_FILES += $$PWD/TODO

SUBDIRS      = $$PWD/modules/modules.pro \
               $$PWD/utils/utils.pro \
               $$PWD/interpreter/interpreter.pro \
               $$PWD/themer/themer.pro \
               $$PWD/updater/updater.pro \
               $$PWD/objectwheel/objectwheel.pro
