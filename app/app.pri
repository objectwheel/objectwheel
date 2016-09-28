INCLUDEPATH = $$PWD/../contrib/fit/include \
              $$PWD/../contrib/miniz/include \
              $$PWD/../contrib/couchbase/include

debug {
    LIBS    = -L$$PWD/../contrib/fit/build/debug \
              -L$$PWD/../contrib/miniz/build/debug \
              -L$$PWD/../contrib/couchbase/build/debug
} release {
	LIBS    = -L$$PWD/../contrib/fit/build/release \
              -L$$PWD/../contrib/miniz/build/release \
              -L$$PWD/../contrib/couchbase/build/release
}

LIBS       += -lfit \
              -lminiz \
              -lcouchbase