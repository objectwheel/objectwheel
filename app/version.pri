VERSION_FILE = $$PWD/../VERSION

# Need to discard STDERR so get path to NULL device
win32 {
    NULL_DEVICE = NUL # Windows doesn't have /dev/null but has NUL
} else {
    NULL_DEVICE = /dev/null
}

BASE_GIT_COMMAND = git --git-dir $$PWD/../.git --work-tree $$PWD/..
GIT_VERSION = $$system($$BASE_GIT_COMMAND describe --always --tags 2> $$NULL_DEVICE)

CONF_VERSION = $$section(GIT_VERSION, -, 0, 0)
CONF_VERSION ~= s/v/""
CONF_VERSION = $$CONF_VERSION-$$section(GIT_VERSION, -, 1, 2)
CONF_VERSION ~= s/-/"."

win32 { # On windows version can only be numerical so remove commit hash
    CONF_VERSION ~= s/\.\d+\.[a-f0-9]{6,}//
}

ZEROS =
COMMIT_VER = $$section(CONF_VERSION, ., 2, 2)
SZVER = $$str_size($$COMMIT_VER)
equals(SZVER, 0) {
    ZEROS = 00
}
equals(SZVER, 1) {
    ZEROS = 0
}

VERSION = $$section(CONF_VERSION, ., 0, 1)$$ZEROS$$COMMIT_VER
GIT_HASH = $$section(CONF_VERSION, ., 3, 4)
GIT_HASH ~= s/g/""
GIT_DATE = $$system($$BASE_GIT_COMMAND log -1 --format=%cd)

equals(GIT_HASH, ) {
    GIT_HASH = master
}

# By default Qt only uses major and minor version for Info.plist on Mac.
# This will rewrite Info.plist with full version
macx {
    PLIST_PATH = $$shell_quote($${OUT_PWD}/$${TARGET}.app/Contents/Info.plist)
    QMAKE_POST_LINK += /usr/libexec/PlistBuddy -c \"Set :CFBundleShortVersionString $${CONF_VERSION}\" -c \"Set :CFBundleInfoDictionaryVersion $${CONF_VERSION}\" $${PLIST_PATH}
}

CONFIG(release, debug | release) {
    !infile($$VERSION_FILE, PREVIOUS_VERSION, $$GIT_VERSION) {
        VERSION_DATA = PREVIOUS_VERSION=$$GIT_VERSION
        write_file($$VERSION_FILE, VERSION_DATA)
        touch($$PWD/aboutwindow.cpp, $$VERSION_FILE)
        touch($$PWD/main.cpp, $$VERSION_FILE)
        touch($$PWD/projectswidget.cpp, $$VERSION_FILE)
    }
}