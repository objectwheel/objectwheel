windows {
    RC_ICONS = $$PWD/windows/icon.ico
}

macx {
    QMAKE_MAC_SDK    = macosx10.10
    QMAKE_INFO_PLIST = $$PWD/mac/Info.plist
    ICON             = $$PWD/mac/icon.icns
#    QMAKE_POST_LINK += macdeployqt $$BUILD_DIR/Objectwheel.app/ -qmldir=$$PWD/src/resources/qmls/ -verbose=1 -dmg
}

android {
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

    DISTFILES += $$PWD/android/build.gradle \
                 $$PWD/android/AndroidManifest.xml \
                 $$PWD/android/res/values/libs.xml \
                 $$PWD/android/res/drawable-mdpi/icon.png \
                 $$PWD/android/res/drawable-ldpi/icon.png \
                 $$PWD/android/res/drawable-hdpi/icon.png
}
