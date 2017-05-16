# Detecting target architecture and setting BUILD_POSTFIX variable with a build path

!build_pass {
    DETECTED_OS = $$first(QMAKE_PLATFORM)

    iphonesimulator {
        DETECTED_ARCH = i386
    } else {
        DETECTED_ARCH = $$QT_ARCH
    }

    CONFIG(debug, debug|release) {
        DETECTED_BUILD = debug
    } else {
        DETECTED_BUILD = release
    }

    BUILD_POSTFIX = $$lower(build/$$DETECTED_OS-$$DETECTED_ARCH-$$DETECTED_BUILD)
}
