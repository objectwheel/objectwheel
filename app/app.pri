INCLUDEPATH += $$PWD/../contrib/fit/include \
			   $$PWD/../contrib/miniz/include \
			   $$PWD/../contrib/couchbase/include

CONFIG(debug, debug|release) {
	BUILD_DIR = build/$$first(QMAKE_PLATFORM)-$$QT_ARCH-debug
} else {
	BUILD_DIR = build/$$first(QMAKE_PLATFORM)-$$QT_ARCH-release
}

LIBS        += -L$$PWD/../contrib/fit/$$BUILD_DIR \
			   -L$$PWD/../contrib/miniz/$$BUILD_DIR \
			   -L$$PWD/../contrib/couchbase/$$BUILD_DIR

LIBS        += -lfit \
			   -lminiz \
			   -lcouchbase
