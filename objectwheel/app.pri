### Utils Settings
INCLUDEPATH += $$PWD/../utils
INCLUDEPATH += $$PWD/../utils/zipasync
INCLUDEPATH += $$PWD/../utils/zipasync/async
INCLUDEPATH += $$PWD/../utils/qmlparser
INCLUDEPATH += $$PWD/../utils/qmlparser/utils
RESOURCES   += $$PWD/../utils/resources/resources.qrc
LIBS        += -L$$OUT_PWD/../utils -lutils
windows:CONFIG(release, debug | release):LIBS += -L$$OUT_PWD/../utils/release
windows:CONFIG(debug, debug | release):LIBS += -L$$OUT_PWD/../utils/debug

### Installs
macx {
    interpreter.files = $$OUT_PWD/../interpreter/interpreter
    interpreter.path = Contents/MacOS
    renderer.files = $$OUT_PWD/../renderer/renderer
    renderer.path = Contents/MacOS
    themer.files = $$OUT_PWD/../themer/themer
    themer.path = Contents/MacOS
    utils.files = $$OUT_PWD/../utils/libutils.dylib
    utils.path = Contents/Frameworks
    QMAKE_POST_LINK += $$system(install_name_tool $$OUT_PWD/Objectwheel.app/Contents/MacOS/Objectwheel -change libutils.dylib @loader_path/../Frameworks/libutils.dylib)
    QMAKE_POST_LINK += $$system(install_name_tool $$OUT_PWD/Objectwheel.app/Contents/MacOS/interpreter -change libutils.dylib @loader_path/../Frameworks/libutils.dylib)
    QMAKE_POST_LINK += $$system(install_name_tool $$OUT_PWD/Objectwheel.app/Contents/MacOS/renderer -change libutils.dylib @loader_path/../Frameworks/libutils.dylib)
    QMAKE_POST_LINK += $$system(install_name_tool $$OUT_PWD/Objectwheel.app/Contents/MacOS/themer -change libutils.dylib @loader_path/../Frameworks/libutils.dylib)
    docs.files = $$PWD/resources/docs
    docs.path = Contents/Resources
    modules.files = $$OUT_PWD/../modules/modules
    modules.path = Contents/Frameworks
    QMAKE_BUNDLE_DATA += interpreter renderer themer utils docs modules
} else:unix {
    interpreter.files = $$OUT_PWD/../interpreter/interpreter
    interpreter.path = $$OUT_PWD/
    renderer.files = $$OUT_PWD/../renderer/renderer
    renderer.path = $$OUT_PWD/
    themer.files = $$OUT_PWD/../themer/themer
    themer.path = $$OUT_PWD/
    utils.files = $$OUT_PWD/../utils/libutils.so
    utils.path = $$OUT_PWD/
    docs.files = $$PWD/resources/docs/*
    docs.path = $$OUT_PWD/docs
    # TODO : Copy modules too
    INSTALLS += interpreter renderer themer utils docs
} else:windows {
    CONFIG(debug, debug | release):COMPILING_MODE = debug
    CONFIG(release, debug | release):COMPILING_MODE = release

    FILES_TO_COPY = $$OUT_PWD/../interpreter/$$COMPILING_MODE/interpreter.exe
    FILES_TO_COPY += $$OUT_PWD/../renderer/$$COMPILING_MODE/renderer.exe
    FILES_TO_COPY += $$OUT_PWD/../themer/$$COMPILING_MODE/themer.exe
    FILES_TO_COPY += $$OUT_PWD/../utils/$$COMPILING_MODE/utils.dll
    DESTINATION_DIR = $$shell_quote($$shell_path($$OUT_PWD/$$COMPILING_MODE))

    for (FILE, FILES_TO_COPY) {
        FILE_PATH = $$shell_quote($$shell_path($$FILE))
        QMAKE_POST_LINK += $$QMAKE_COPY $$FILE_PATH $$DESTINATION_DIR $$escape_expand(\n\t)
    }

    DIRS_TO_COPY = $$PWD/resources/docs
    DESTINATION_DIR = $$shell_quote($$shell_path($$OUT_PWD/$$COMPILING_MODE/docs))

    for (DIR, DIRS_TO_COPY) {
        DIR_PATH = $$shell_quote($$shell_path($$DIR))
        QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$DIR_PATH $$DESTINATION_DIR $$escape_expand(\n\t)
    }
    # TODO : Copy modules too
}
