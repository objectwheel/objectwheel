#include <qmleditorview.h>
#include <formscene.h>
#include <control.h>
#include <fit.h>
#include <savemanager.h>
#include <designmanager.h>
#include <filemanager.h>
#include <qmltexteditor.h>
#include <css.h>

#include <QDebug>
#include <QVBoxLayout>
#include <QPainter>
#include <QToolBar>
#include <QAction>
#include <QToolButton>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>

#define LINE_COLOR ("#606467")
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

    public slots:
        void handleCursorPositionChanged();
        void handlePinButtonClicked();

    public:
        QmlEditorView* parent;
        QVBoxLayout vBoxLayout;
        QWidget containerWidget;
        QVBoxLayout containerVBoxLayout;
        QmlTextEditor textEditor;
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

};

QmlEditorViewPrivate::QmlEditorViewPrivate(QmlEditorView* parent)
    : QObject(parent)
    , parent(parent)
    , vBoxLayout(parent)
    , containerVBoxLayout(&containerWidget)
{
    vBoxLayout.setContentsMargins(0, 0, 0, 0);
    vBoxLayout.setSpacing(0);
    vBoxLayout.addWidget(&containerWidget);

    containerVBoxLayout.setContentsMargins(0, 0, 0, 0);
    containerVBoxLayout.setSpacing(0);
    containerVBoxLayout.addWidget(&toolbar);
    containerVBoxLayout.addWidget(&textEditor);

    containerWidget.setWindowIcon(QIcon(":/resources/images/owicon.png"));
    containerWidget.setWindowTitle("Objectwheel Qml Editor");

    QFont font;
    font.setFamily("Liberation Mono");
    font.setStyleHint(QFont::Monospace);
    textEditor.setFont(font);
    textEditor.setText(TEST_TEXT);
    textEditor.setStyleSheet("background: #f3f6f9;");
    textEditor.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(&textEditor, SIGNAL(cursorPositionChanged()), SLOT(handleCursorPositionChanged()));
    connect(&pinButton, SIGNAL(clicked(bool)), SLOT(handlePinButtonClicked()));

    zoomlLevelCombobox.addItem("25 %");
    zoomlLevelCombobox.addItem("50 %");
    zoomlLevelCombobox.addItem("75 %");
    zoomlLevelCombobox.addItem("100 %");
    zoomlLevelCombobox.addItem("125 %");
    zoomlLevelCombobox.addItem("150 %");
    zoomlLevelCombobox.addItem("175 %");
    zoomlLevelCombobox.addItem("200 %");
    zoomlLevelCombobox.addItem("300 %");
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

    pinButton.setToolTip("Unpin Editor");
    backButton.setToolTip("Go cursor back");
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

    toolbar.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
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
    toolbar.addWidget(&saveButton);
    toolbar.addWidget(&cutButton);
    toolbar.addWidget(&copyButton);
    toolbar.addWidget(&pasteButton);
    toolbar.addSeparator();
    toolbar.addWidget(&closeButton);
    toolbar.addWidget(&documentsCombobox);
    toolbar.addWidget(&zoomlLevelCombobox);
    toolbar.addWidget(spacer);
    toolbar.addWidget(&lineColLabel);
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
