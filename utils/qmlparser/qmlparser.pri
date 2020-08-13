INCLUDEPATH += $$PWD
include(languageutils/languageutils-lib.pri)
include(qmljs/qmljs-lib.pri)
include(utils/utils-lib.pri)
DEFINES += QML_BUILD_STATIC_LIB \
           LANGUAGEUTILS_BUILD_STATIC_LIB \
           QTCREATOR_UTILS_BUILD_STATIC_LIB \
           QML_PARSER_LIBRARY_BUILD_STATIC_LIB
