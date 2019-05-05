CONFIG      += ordered
TEMPLATE     = subdirs

OTHER_FILES += $$PWD/TODO

SUBDIRS      = $$PWD/utils/utils.pro \
               $$PWD/interpreter/interpreter.pro \
               $$PWD/previewer/previewer.pro \
               $$PWD/themer/themer.pro \
               $$PWD/app/app.pro

