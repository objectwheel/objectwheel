CONFIG  += ordered
TEMPLATE = subdirs

SUBDIRS  = $$PWD/utils/utils.pro \
           $$PWD/objectwheel-interpreter/objectwheel-interpreter.pro \
           $$PWD/objectwheel-previewer/objectwheel-previewer.pro \
           $$PWD/objectwheel-themer/objectwheel-themer.pro \
           $$PWD/app/app.pro
