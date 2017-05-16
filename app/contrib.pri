INCLUDEPATH     += $$PROJECT_ROOT/contrib/fit/include \
				   $$PROJECT_ROOT/contrib/miniz/include \
				   $$PROJECT_ROOT/contrib/aes/include

windows {
LIBS            += -L$$PROJECT_ROOT/contrib/fit \
                                   -L$$PROJECT_ROOT/contrib/miniz \
                                   -L$$PROJECT_ROOT/contrib/aes
} else {
LIBS            += -L$$PROJECT_ROOT/contrib/fit/$$BUILD_POSTFIX \
                                   -L$$PROJECT_ROOT/contrib/miniz/$$BUILD_POSTFIX \
                                   -L$$PROJECT_ROOT/contrib/aes/$$BUILD_POSTFIX
}
LIBS            += -lfit \
				   -lminiz \
				   -laes

include($$PWD/../contrib/qmlparser/qmlparser.pri)
