INCLUDEPATH     += $$PROJECT_ROOT/contrib/fit/include \
                   $$PROJECT_ROOT/contrib/miniz/include

ios | windows {
    LIBS        += -L$$PROJECT_ROOT/contrib/fit \
                   -L$$PROJECT_ROOT/contrib/miniz
} else {
    LIBS        += -L$$PROJECT_ROOT/contrib/fit/$$BUILD_POSTFIX \
                   -L$$PROJECT_ROOT/contrib/miniz/$$BUILD_POSTFIX
}

LIBS            += -lfit \
                   -lminiz
