#include <qmleditorview.h>
#include <formscene.h>
#include <control.h>
#include <fit.h>
#include <savemanager.h>
#include <designmanager.h>
#include <filemanager.h>
#include <qmlcodeeditor.h>
#include <css.h>
#include <fileexplorer.h>

#include <QDebug>
#include <QVBoxLayout>
#include <QPainter>
#include <QToolBar>
#include <QAction>
#include <QToolButton>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QSplitter>

#define LINE_COLOR ("#606467")
#define INITIALWIDTH_FILEEXPLORER (fit(450))
#define MINWIDTH_FILEEXPLORER (fit(200))
#define MINWIDTH_EDITOR (fit(200))

#define TEST_TEXT "#include <QtGui>\n"\
"\n"\
"int main(int argv, char** argc)\n"\
"{\n"\
"    QApplication a(argv, argc);\n"\
"\n"\
"    QPushButton(\"Hello World\").show();\n"\
"\n"\
"    return a.exec();\n"\
"}\n"

using namespace Fit;

class QmlEditorViewPrivate : public QObject
{
        Q_OBJECT

    public:
        QmlEditorViewPrivate(QmlEditorView* parent);

    private:
        int findRatio(const QString& text);

    public slots:
        void handleCursorPositionChanged();
        void handlePinButtonClicked();
        void handleZoomLevelChange(const QString& text);
        void handleHideShowButtonClicked();

    public:
        QmlEditorView* parent;
        QVBoxLayout vBoxLayout;
        QWidget containerWidget;
        QVBoxLayout containerVBoxLayout;

        QToolBar toolbar;
        QToolButton pinButton;
        QToolButton undoButton;
        QToolButton redoButton;
        QToolButton backButton;
        QToolButton forthButton;
        QToolButton closeButton;
        QToolButton saveButton;
        QToolButton cutButton;
        QToolButton copyButton;
        QToolButton pasteButton;
        QComboBox documentsCombobox;
        QComboBox zoomlLevelCombobox;
        QLabel lineColLabel;
        int previousRatio;

        QSplitter splitter;
        QWidget editorWrapper;
        QVBoxLayout editorWrapperVBoxLayout;
        QmlCodeEditor textEditor;
        QWidget explorerWrapper;
        QHBoxLayout explorerWrapperHBoxLayout;
        QToolBar toolbar_2;
        QToolButton hideShowButton;
        QToolButton textEditorButton;
        QToolButton imageEditorButton;
        QToolButton hexEditorButton;
        FileExplorer fileExplorer;
        int lastWidthOfExplorerWrapper;
};

QmlEditorViewPrivate::QmlEditorViewPrivate(QmlEditorView* parent)
    : QObject(parent)
    , parent(parent)
    , vBoxLayout(parent)
    , containerVBoxLayout(&containerWidget)
    , previousRatio(0)
    , editorWrapperVBoxLayout(&editorWrapper)
    , explorerWrapperHBoxLayout(&explorerWrapper)
    , lastWidthOfExplorerWrapper(INITIALWIDTH_FILEEXPLORER)
{
    vBoxLayout.setContentsMargins(0, 0, 0, 0);
    vBoxLayout.setSpacing(0);
    vBoxLayout.addWidget(&containerWidget);

    containerVBoxLayout.setContentsMargins(0, 0, 0, 0);
    containerVBoxLayout.setSpacing(0);
    containerVBoxLayout.addWidget(&toolbar);
    containerVBoxLayout.addWidget(&splitter);

    splitter.setStyleSheet("QSplitter{background: #e0e4e7;}");
    splitter.addWidget(&editorWrapper);
    splitter.addWidget(&explorerWrapper);
    splitter.setCollapsible(0, false);;
    splitter.setCollapsible(1, false);;

    editorWrapperVBoxLayout.setContentsMargins(0, 0, 0, 0);
    editorWrapperVBoxLayout.setSpacing(0);
    editorWrapperVBoxLayout.addWidget(&textEditor);

    explorerWrapperHBoxLayout.setContentsMargins(0, 0, 0, 0);
    explorerWrapperHBoxLayout.setSpacing(0);
    explorerWrapperHBoxLayout.addWidget(&toolbar_2);
    explorerWrapperHBoxLayout.addWidget(&fileExplorer);

    fileExplorer.hide();
    fileExplorer.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    containerWidget.setWindowTitle("Objectwheel Qml Editor");

    redoButton.setDisabled(true);
    copyButton.setDisabled(true);
    cutButton.setDisabled(true);

    QFont font;
    font.setFamily("Liberation Mono");
    font.setStyleHint(QFont::Monospace);
    textEditor.setFont(font);
    textEditor.setPlainText(TEST_TEXT);
    textEditor.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(&textEditor, SIGNAL(cursorPositionChanged()), SLOT(handleCursorPositionChanged()));
    connect(&pinButton, SIGNAL(clicked(bool)), SLOT(handlePinButtonClicked()));
    connect(&undoButton, SIGNAL(clicked(bool)), &textEditor, SLOT(undo()));
    connect(&redoButton, SIGNAL(clicked(bool)), &textEditor, SLOT(redo()));
    connect(&textEditor, SIGNAL(undoAvailable(bool)), &undoButton, SLOT(setEnabled(bool)));
    connect(&textEditor, SIGNAL(redoAvailable(bool)), &redoButton, SLOT(setEnabled(bool)));
    connect(&copyButton, SIGNAL(clicked(bool)), &textEditor, SLOT(copy()));
    connect(&cutButton, SIGNAL(clicked(bool)), &textEditor, SLOT(cut()));
    connect(&pasteButton, SIGNAL(clicked(bool)), &textEditor, SLOT(paste()));
    connect(&textEditor, SIGNAL(copyAvailable(bool)), &copyButton, SLOT(setEnabled(bool)));
    connect(&textEditor, SIGNAL(copyAvailable(bool)), &cutButton, SLOT(setEnabled(bool)));
    connect(&zoomlLevelCombobox, SIGNAL(currentTextChanged(QString)), SLOT(handleZoomLevelChange(QString)));
    connect(&hideShowButton, SIGNAL(clicked(bool)), SLOT(handleHideShowButtonClicked()));

    zoomlLevelCombobox.addItem("25 %");
    zoomlLevelCombobox.addItem("50 %");
    zoomlLevelCombobox.addItem("75 %");
    zoomlLevelCombobox.addItem("100 %");
    zoomlLevelCombobox.addItem("125 %");
    zoomlLevelCombobox.addItem("150 %");
    zoomlLevelCombobox.addItem("175 %");
    zoomlLevelCombobox.addItem("200 %");
    zoomlLevelCombobox.addItem("300 %");
    zoomlLevelCombobox.addItem("500 %");
    zoomlLevelCombobox.addItem("900 %");
    zoomlLevelCombobox.setCurrentIndex(3);

    pinButton.setCursor(Qt::PointingHandCursor);
    backButton.setCursor(Qt::PointingHandCursor);
    forthButton.setCursor(Qt::PointingHandCursor);
    undoButton.setCursor(Qt::PointingHandCursor);
    redoButton.setCursor(Qt::PointingHandCursor);
    closeButton.setCursor(Qt::PointingHandCursor);
    zoomlLevelCombobox.setCursor(Qt::PointingHandCursor);
    documentsCombobox.setCursor(Qt::PointingHandCursor);
    saveButton.setCursor(Qt::PointingHandCursor);
    cutButton.setCursor(Qt::PointingHandCursor);
    copyButton.setCursor(Qt::PointingHandCursor);
    pasteButton.setCursor(Qt::PointingHandCursor);

    pinButton.setToolTip("Unpin Editor.");
    backButton.setToolTip("Go cursor back.");
    forthButton.setToolTip("Go cursor forth.");
    undoButton.setToolTip("Undo action.");
    redoButton.setToolTip("Redo action.");
    closeButton.setToolTip("Close document.");
    zoomlLevelCombobox.setToolTip("Change zoom level.");
    documentsCombobox.setToolTip("Change document.");
    lineColLabel.setToolTip("Cursor line/column indicator.");
    saveButton.setToolTip("Save document.");
    cutButton.setToolTip("Cut selection.");
    copyButton.setToolTip("Copy selection.");
    pasteButton.setToolTip("Paste from clipboard.");

    pinButton.setIcon(QIcon(":/resources/images/unpin.png"));
    backButton.setIcon(QIcon(":/resources/images/dback.png"));
    forthButton.setIcon(QIcon(":/resources/images/dforth.png"));
    undoButton.setIcon(QIcon(":/resources/images/undo.png"));
    redoButton.setIcon(QIcon(":/resources/images/redo.png"));
    closeButton.setIcon(QIcon(":/resources/images/delete-icon.png"));
    saveButton.setIcon(QIcon(":/resources/images/save.png"));
    cutButton.setIcon(QIcon(":/resources/images/cut.png"));
    copyButton.setIcon(QIcon(":/resources/images/copy.png"));
    pasteButton.setIcon(QIcon(":/resources/images/paste.png"));

    QWidget* spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    toolbar.setStyleSheet(CSS::DesignerToolbar);
    toolbar.setIconSize(QSize(fit(14), fit(14)));
    toolbar.setFixedHeight(fit(26));
    toolbar.addWidget(&pinButton);
    toolbar.addSeparator();
    toolbar.addWidget(&undoButton);
    toolbar.addWidget(&redoButton);
    toolbar.addSeparator();
    toolbar.addWidget(&backButton);
    toolbar.addWidget(&forthButton);
    toolbar.addSeparator();
    toolbar.addWidget(&cutButton);
    toolbar.addWidget(&copyButton);
    toolbar.addWidget(&pasteButton);
    toolbar.addWidget(&saveButton);
    toolbar.addSeparator();
    toolbar.addWidget(&closeButton);
    toolbar.addWidget(&documentsCombobox);
    toolbar.addWidget(&zoomlLevelCombobox);
    toolbar.addWidget(spacer);
    toolbar.addWidget(&lineColLabel);

    // Toolbar_2 assets
    hideShowButton.setCursor(Qt::PointingHandCursor);
    textEditorButton.setCursor(Qt::PointingHandCursor);
    imageEditorButton.setCursor(Qt::PointingHandCursor);
    hexEditorButton.setCursor(Qt::PointingHandCursor);

    hideShowButton.setToolTip("Show File Explorer.");
    textEditorButton.setToolTip("Open Text Editor.");
    imageEditorButton.setToolTip("Open Image Viewer.");
    hexEditorButton.setToolTip("Open Hex Editor.");

    hideShowButton.setIcon(QIcon(":/resources/images/show.png"));
    textEditorButton.setIcon(QIcon(":/resources/images/code.png"));
    imageEditorButton.setIcon(QIcon(":/resources/images/image.png"));
    hexEditorButton.setIcon(QIcon(":/resources/images/hex.png"));

    toolbar_2.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    toolbar_2.setOrientation(Qt::Vertical);
    toolbar_2.setStyleSheet(CSS::DesignerToolbarV);
    toolbar_2.setIconSize(QSize(fit(14), fit(14)));
    toolbar_2.setFixedWidth(fit(26));
    toolbar_2.addWidget(&hideShowButton);
    toolbar_2.addSeparator();
    toolbar_2.addWidget(&textEditorButton);
    toolbar_2.addWidget(&imageEditorButton);
    toolbar_2.addWidget(&hexEditorButton);

    splitter.handle(1)->setDisabled(true);
    editorWrapper.setMinimumWidth(MINWIDTH_FILEEXPLORER);
    explorerWrapper.setFixedWidth(toolbar_2.width());
    connect(&splitter, &QSplitter::splitterMoved, [=] {
        lastWidthOfExplorerWrapper = explorerWrapper.width();
    });
}

int QmlEditorViewPrivate::findRatio(const QString& text)
{
    if (text == "25 %")
        return -4;
    else if (text == "50 %")
        return -2;
    else if (text == "75 %")
        return -1;
    else if (text == "100 %")
        return 0;
    else if (text == "125 %")
        return 1;
    else if (text == "150 %")
        return 2;
    else if (text == "175 %")
        return 3;
    else if (text == "200 %")
        return 4;
    else if (text == "300 %")
        return 5;
    else if (text == "500 %")
        return 7;
    else if (text == "900 %")
        return 9;
    else
        return 0;
}

void QmlEditorViewPrivate::handleCursorPositionChanged()
{
    auto textCursor = textEditor.textCursor();
    QString lineColText("# Line: %1, Col: %2");
    lineColLabel.setText(lineColText.arg(textCursor.blockNumber() + 1).arg(textCursor.columnNumber()));
}

void QmlEditorViewPrivate::handlePinButtonClicked()
{
    if (pinButton.toolTip().contains("Unpin")) {
        pinButton.setToolTip("Pin Editor.");
        pinButton.setIcon(QIcon(":/resources/images/pin.png"));
        containerWidget.setParent(nullptr);
        containerWidget.show();
        containerWidget.setWindowIcon(QIcon(":/resources/images/owicon.png"));
    } else {
        pinButton.setToolTip("Unpin Editor.");
        pinButton.setIcon(QIcon(":/resources/images/unpin.png"));
        vBoxLayout.addWidget(&containerWidget);
    }
}

void QmlEditorViewPrivate::handleZoomLevelChange(const QString& text)
{
    int ratio = findRatio(text);

    if (previousRatio > 0)
        textEditor.zoomOut(previousRatio);
    else
        textEditor.zoomIn(-previousRatio);

    if (ratio > 0)
        textEditor.zoomIn(ratio);
    else
        textEditor.zoomOut(-ratio);

    previousRatio = ratio;

}

void QmlEditorViewPrivate::handleHideShowButtonClicked()
{
    if (hideShowButton.toolTip().contains("Hide")) {
        hideShowButton.setIcon(QIcon(":/resources/images/show.png"));
        hideShowButton.setToolTip("Show File Explorer.");
        splitter.handle(1)->setDisabled(true);
        fileExplorer.hide();
        explorerWrapper.setFixedWidth(toolbar_2.width());
        QList<int> sizes;
        sizes << containerWidget.width() - toolbar_2.width();
        sizes << toolbar_2.width();
        splitter.setSizes(sizes);
    } else {
        hideShowButton.setIcon(QIcon(":/resources/images/hide.png"));
        hideShowButton.setToolTip("Hide File Explorer.");
        splitter.handle(1)->setEnabled(true);
        fileExplorer.show();
        explorerWrapper.setMinimumWidth(MINWIDTH_FILEEXPLORER);
        explorerWrapper.setMaximumWidth(9999);
        QList<int> sizes;
        sizes << containerWidget.width() - lastWidthOfExplorerWrapper;
        sizes << lastWidthOfExplorerWrapper;
        splitter.setSizes(sizes);
    }
}

QmlEditorView::QmlEditorView(QWidget* parent)
    : QWidget(parent)
    , _d(new QmlEditorViewPrivate(this))
{
}

void QmlEditorView::paintEvent(QPaintEvent*)
{
    QPen pen;
    QPainter painter(this);
    QRectF _rect(0, 0, fit(150), fit(60));
    _rect.moveCenter(rect().center());
    pen.setStyle(Qt::DotLine);
    pen.setColor(LINE_COLOR);
    painter.setPen(pen);
    painter.drawRect(_rect);
    painter.drawText(_rect, "Editor unpinned, pin it again.", QTextOption(Qt::AlignCenter));
}

#include "qmleditorview.moc"
