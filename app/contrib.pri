INCLUDEPATH     += $$PROJECT_ROOT/contrib/fit/include \
                   $$PROJECT_ROOT/contrib/miniz/include \
                   $$PROJECT_ROOT/contrib/couchbase/include

ios {
    LIBS        += -L$$PROJECT_ROOT/contrib/fit \
                   -L$$PROJECT_ROOT/contrib/miniz \
                   -L$$PROJECT_ROOT/contrib/couchbase
} else {
    LIBS        += -L$$PROJECT_ROOT/contrib/fit/$$BUILD_POSTFIX \
                   -L$$PROJECT_ROOT/contrib/miniz/$$BUILD_POSTFIX \
                   -L$$PROJECT_ROOT/contrib/couchbase/$$BUILD_POSTFIX
}

LIBS            += -lfit \
                   -lminiz \
                   -lcouchbase
