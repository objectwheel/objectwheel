### Copy qmldir file and then generate plugins.qmltypes file

FILES_TO_COPY = $$_PRO_FILE_PWD_/qmldir
DESTINATION_DIR = $$shell_quote($$shell_path($$OUT_PWD/$$DESTDIR))

for (FILE, FILES_TO_COPY) {
    FILE_PATH = $$shell_quote($$shell_path($$FILE))
    QMAKE_POST_LINK += $$QMAKE_COPY $$FILE_PATH $$DESTINATION_DIR $$escape_expand(\n\t)
}

# qmltypes files are only needed by intellisense of Objectwheel
macx:windows:linux {
    QMAKE_POST_LINK += qmlplugindump -nonrelocatable $$basename(DESTDIR) $$VERSION \
                       $$shell_quote($$shell_path($$OUT_PWD/../Modules)) > \
                       $$shell_quote($$shell_path($$OUT_PWD/$$DESTDIR/plugins.qmltypes))
}
