### Utils Settings
INCLUDEPATH += $$PWD/../utils
INCLUDEPATH += $$PWD/../utils/zipasync
INCLUDEPATH += $$PWD/../utils/zipasync/async
INCLUDEPATH += $$PWD/../utils/qmlparser
INCLUDEPATH += $$PWD/../utils/qmlparser/utils
RESOURCES   += $$PWD/../utils/resources/resources.qrc
LIBS        += -L$$OUT_PWD/../utils -lUtils
windows:CONFIG(release, debug | release):LIBS += -L$$OUT_PWD/../utils/release
windows:CONFIG(debug, debug | release):LIBS += -L$$OUT_PWD/../utils/debug

### Installs
macx {
    interpreter.files = $$OUT_PWD/../interpreter/Interpreter
    interpreter.path = Contents/MacOS
    renderer.files = $$OUT_PWD/../renderer/Renderer
    renderer.path = Contents/MacOS
    themer.files = $$OUT_PWD/../themer/Themer
    themer.path = Contents/MacOS
    updater.files = $$OUT_PWD/../updater/Updater
    updater.path = Contents/MacOS
    utils.files = $$OUT_PWD/../utils/libUtils.dylib
    utils.path = Contents/Frameworks
    QMAKE_POST_LINK += install_name_tool $$OUT_PWD/Objectwheel.app/Contents/MacOS/Objectwheel -change \
                       libUtils.dylib @loader_path/../Frameworks/libUtils.dylib $$escape_expand(\n\t)
    QMAKE_POST_LINK += install_name_tool $$OUT_PWD/Objectwheel.app/Contents/MacOS/Interpreter -change \
                       libUtils.dylib @loader_path/../Frameworks/libUtils.dylib $$escape_expand(\n\t)
    QMAKE_POST_LINK += install_name_tool $$OUT_PWD/Objectwheel.app/Contents/MacOS/Renderer -change \
                       libUtils.dylib @loader_path/../Frameworks/libUtils.dylib $$escape_expand(\n\t)
    QMAKE_POST_LINK += install_name_tool $$OUT_PWD/Objectwheel.app/Contents/MacOS/Themer -change \
                       libUtils.dylib @loader_path/../Frameworks/libUtils.dylib $$escape_expand(\n\t)
    docs.files = $$PWD/resources/Documents
    docs.path = Contents/Resources
    modules.files = $$OUT_PWD/../modules/Modules
    modules.path = Contents/Resources
    QMAKE_BUNDLE_DATA += interpreter renderer themer updater utils docs modules
} else:unix {
    interpreter.files = $$OUT_PWD/../interpreter/Interpreter
    interpreter.path = $$OUT_PWD/
    renderer.files = $$OUT_PWD/../renderer/Renderer
    renderer.path = $$OUT_PWD/
    themer.files = $$OUT_PWD/../themer/Themer
    themer.path = $$OUT_PWD/
    updater.files = $$OUT_PWD/../updater/Updater
    updater.path = $$OUT_PWD/
    utils.files = $$OUT_PWD/../utils/libUtils.so
    utils.path = $$OUT_PWD/
    docs.files = $$PWD/resources/Documents/*
    docs.path = $$OUT_PWD/Documents
    # TODO : Copy modules too
    INSTALLS += interpreter renderer updater themer utils docs
} else:windows {
    CONFIG(debug, debug | release):COMPILING_MODE = debug
    CONFIG(release, debug | release):COMPILING_MODE = release

    FILES_TO_COPY = $$OUT_PWD/../interpreter/$$COMPILING_MODE/Interpreter.exe
    FILES_TO_COPY += $$OUT_PWD/../renderer/$$COMPILING_MODE/Renderer.exe
    FILES_TO_COPY += $$OUT_PWD/../themer/$$COMPILING_MODE/Themer.exe
    FILES_TO_COPY += $$OUT_PWD/../updater/$$COMPILING_MODE/Updater.exe
    FILES_TO_COPY += $$OUT_PWD/../utils/$$COMPILING_MODE/Utils.dll
    DESTINATION_DIR = $$shell_quote($$shell_path($$OUT_PWD/$$COMPILING_MODE))

    for (FILE, FILES_TO_COPY) {
        FILE_PATH = $$shell_quote($$shell_path($$FILE))
        QMAKE_POST_LINK += $$QMAKE_COPY $$FILE_PATH $$DESTINATION_DIR $$escape_expand(\n\t)
    }

    DIRS_TO_COPY = $$PWD/resources/Documents
    DESTINATION_DIR = $$shell_quote($$shell_path($$OUT_PWD/$$COMPILING_MODE/Documents))

    for (DIR, DIRS_TO_COPY) {
        DIR_PATH = $$shell_quote($$shell_path($$DIR))
        QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$DIR_PATH $$DESTINATION_DIR $$escape_expand(\n\t)
    }
    # TODO : Copy modules too
}
