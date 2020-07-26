#include <menumanager.h>
#include <windowmanager.h>
#include <mainwindow.h>
#include <preferenceswindow.h>
#include <aboutwindow.h>
#include <appconstants.h>

#include <QMenuBar>
#include <QApplication>

QMenuBar* MenuManager::s_menuBar = nullptr;
QMenu* MenuManager::s_fileMenu = nullptr;
QMenu* MenuManager::s_editMenu = nullptr;
QMenu* MenuManager::s_formatMenu = nullptr;
QMenu* MenuManager::s_helpMenu = nullptr;
QMenu* MenuManager::s_optionsMenu = nullptr;
QActionGroup* MenuManager::s_alignmentGroup = nullptr;
QAction* MenuManager::s_newAct = nullptr;
QAction* MenuManager::s_openAct = nullptr;
QAction* MenuManager::s_saveAct = nullptr;
QAction* MenuManager::s_printAct = nullptr;
QAction* MenuManager::s_exitAct = nullptr;
QAction* MenuManager::s_undoAct = nullptr;
QAction* MenuManager::s_redoAct = nullptr;
QAction* MenuManager::s_cutAct = nullptr;
QAction* MenuManager::s_copyAct = nullptr;
QAction* MenuManager::s_pasteAct = nullptr;
QAction* MenuManager::s_boldAct = nullptr;
QAction* MenuManager::s_italicAct = nullptr;
QAction* MenuManager::s_leftAlignAct = nullptr;
QAction* MenuManager::s_rightAlignAct = nullptr;
QAction* MenuManager::s_justifyAct = nullptr;
QAction* MenuManager::s_centerAct = nullptr;
QAction* MenuManager::s_setLineSpacingAct = nullptr;
QAction* MenuManager::s_setParagraphSpacingAct = nullptr;
QAction* MenuManager::s_preferencesAct = nullptr;
QAction* MenuManager::s_aboutAct = nullptr;
QAction* MenuManager::s_aboutQtAct = nullptr;

MenuManager::MenuManager(QObject* parent) : QObject(parent)
{
    s_menuBar = WindowManager::mainWindow()->menuBar();
    s_menuBar->setStyleSheet("QMenuBar {"
                             "    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
                             "    stop:0 #ffffff, stop:1 #d6d9dc);"
                             "}");

    // FIXME: We will come back that issue later.
    // Each window on window manager will have a separate window
    // createActions(this);
    // createMenus();
}

void MenuManager::createActions(QObject* parent)
{
    s_newAct = new QAction(tr("&New"), parent);
    s_newAct->setShortcuts(QKeySequence::New);
    s_newAct->setStatusTip(tr("Create a new file"));
    connect(s_newAct, &QAction::triggered, &MenuManager::onNewFile);

    s_openAct = new QAction(tr("&Open..."), parent);
    s_openAct->setShortcuts(QKeySequence::Open);
    s_openAct->setStatusTip(tr("Open an existing file"));
    connect(s_openAct, &QAction::triggered, &MenuManager::onOpen);

    s_saveAct = new QAction(tr("&Save"), parent);
    //    s_saveAct->setShortcuts(QKeySequence::Save);
    s_saveAct->setStatusTip(tr("Save the document to disk"));
    connect(s_saveAct, &QAction::triggered, &MenuManager::onSave);

    s_printAct = new QAction(tr("&Print..."), parent);
    s_printAct->setShortcuts(QKeySequence::Print);
    s_printAct->setStatusTip(tr("Print the document"));
    connect(s_printAct, &QAction::triggered, &MenuManager::onPrint);

    s_exitAct = new QAction(tr("E&xit"), parent);
    s_exitAct->setShortcuts(QKeySequence::Quit);
    s_exitAct->setStatusTip(tr("Exit the application"));
    // TODO: connect(s_exitAct, &QAction::triggered, &QWidget::close);

    s_undoAct = new QAction(tr("&Undo"), parent);
    s_undoAct->setShortcuts(QKeySequence::Undo);
    s_undoAct->setStatusTip(tr("Undo the last operation"));
    connect(s_undoAct, &QAction::triggered, &MenuManager::onUndo);

    s_redoAct = new QAction(tr("&Redo"), parent);
    s_redoAct->setShortcuts(QKeySequence::Redo);
    s_redoAct->setStatusTip(tr("Redo the last operation"));
    connect(s_redoAct, &QAction::triggered, &MenuManager::onRedo);

    s_cutAct = new QAction(tr("Cu&t"), parent);
    s_cutAct->setShortcuts(QKeySequence::Cut);
    s_cutAct->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
    connect(s_cutAct, &QAction::triggered, &MenuManager::onCut);

    s_copyAct = new QAction(tr("&Copy"), parent);
    s_copyAct->setShortcuts(QKeySequence::Copy);
    s_copyAct->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
    connect(s_copyAct, &QAction::triggered, &MenuManager::onCopy);

    s_pasteAct = new QAction(tr("&Paste"), parent);
    s_pasteAct->setShortcuts(QKeySequence::Paste);
    s_pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
    connect(s_pasteAct, &QAction::triggered, &MenuManager::onPaste);

    s_boldAct = new QAction(tr("&Bold"), parent);
    s_boldAct->setCheckable(true);
    s_boldAct->setShortcut(QKeySequence::Bold);
    s_boldAct->setStatusTip(tr("Make the text bold"));
    connect(s_boldAct, &QAction::triggered, &MenuManager::onBold);

    QFont boldFont = s_boldAct->font();
    boldFont.setBold(true);
    s_boldAct->setFont(boldFont);

    s_italicAct = new QAction(tr("&Italic"), parent);
    s_italicAct->setCheckable(true);
    s_italicAct->setShortcut(QKeySequence::Italic);
    s_italicAct->setStatusTip(tr("Make the text italic"));
    connect(s_italicAct, &QAction::triggered, &MenuManager::onItalic);

    QFont italicFont = s_italicAct->font();
    italicFont.setItalic(true);
    s_italicAct->setFont(italicFont);

    s_setLineSpacingAct = new QAction(tr("Set &Line Spacing..."), parent);
    s_setLineSpacingAct->setStatusTip(tr("Change the gap between the lines of a paragraph"));
    connect(s_setLineSpacingAct, &QAction::triggered, &MenuManager::onSetLineSpacing);

    s_setParagraphSpacingAct = new QAction(tr("Set &Paragraph Spacing..."), parent);
    s_setParagraphSpacingAct->setStatusTip(tr("Change the gap between paragraphs"));
    connect(s_setParagraphSpacingAct, &QAction::triggered, &MenuManager::onSetParagraphSpacing);

    s_aboutAct = new QAction(tr("&About"), parent);
    s_aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(s_aboutAct, &QAction::triggered, &MenuManager::onAbout);

    s_aboutQtAct = new QAction(tr("About &Qt"), parent);
    s_aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(s_aboutQtAct, &QAction::triggered, qApp, &QApplication::aboutQt);

    s_preferencesAct = new QAction(tr("&Preferences"), parent);
    s_preferencesAct->setStatusTip(tr("Show %1 Preferences").arg(AppConstants::NAME));
    connect(s_preferencesAct, &QAction::triggered, &MenuManager::onPreferences);

    s_leftAlignAct = new QAction(tr("&Left Align"), parent);
    s_leftAlignAct->setCheckable(true);
    s_leftAlignAct->setShortcut(tr("Ctrl+L"));
    s_leftAlignAct->setStatusTip(tr("Left align the selected text"));
    connect(s_leftAlignAct, &QAction::triggered, &MenuManager::onLeftAlign);

    s_rightAlignAct = new QAction(tr("&Right Align"), parent);
    s_rightAlignAct->setCheckable(true);
    s_rightAlignAct->setShortcut(tr("Ctrl+R"));
    s_rightAlignAct->setStatusTip(tr("Right align the selected text"));
    connect(s_rightAlignAct, &QAction::triggered, &MenuManager::onRightAlign);

    s_justifyAct = new QAction(tr("&Justify"), parent);
    s_justifyAct->setCheckable(true);
    s_justifyAct->setShortcut(tr("Ctrl+J"));
    s_justifyAct->setStatusTip(tr("Justify the selected text"));
    connect(s_justifyAct, &QAction::triggered, &MenuManager::onJustify);

    s_centerAct = new QAction(tr("&Center"), parent);
    s_centerAct->setCheckable(true);
    s_centerAct->setShortcut(tr("Ctrl+E"));
    s_centerAct->setStatusTip(tr("Center the selected text"));
    connect(s_centerAct, &QAction::triggered, &MenuManager::onCenter);

    s_alignmentGroup = new QActionGroup(parent);
    s_alignmentGroup->addAction(s_leftAlignAct);
    s_alignmentGroup->addAction(s_rightAlignAct);
    s_alignmentGroup->addAction(s_justifyAct);
    s_alignmentGroup->addAction(s_centerAct);
    s_leftAlignAct->setChecked(true);
}

void MenuManager::createMenus()
{
    s_fileMenu = s_menuBar->addMenu(tr("&File"));
    s_fileMenu->addAction(s_newAct);
    s_fileMenu->addAction(s_openAct);
    s_fileMenu->addAction(s_saveAct);
    s_fileMenu->addAction(s_printAct);
    s_fileMenu->addSeparator();
    s_fileMenu->addAction(s_exitAct);

    s_editMenu = s_menuBar->addMenu(tr("&Edit"));
    s_editMenu->addAction(s_undoAct);
    s_editMenu->addAction(s_redoAct);
    s_editMenu->addSeparator();
    s_editMenu->addAction(s_cutAct);
    s_editMenu->addAction(s_copyAct);
    s_editMenu->addAction(s_pasteAct);
    s_editMenu->addSeparator();

    s_optionsMenu = s_menuBar->addMenu(tr("&Options"));

    s_helpMenu = s_menuBar->addMenu(tr("&Help"));
    s_helpMenu->addAction(s_aboutAct);
    s_helpMenu->addAction(s_aboutQtAct);


#if defined (Q_OS_DARWIN)
    s_helpMenu->addSeparator();
    s_helpMenu->addAction(s_preferencesAct);
#else
    s_optionsMenu->addAction(s_preferencesAct);
#endif

    s_formatMenu = s_editMenu->addMenu(tr("&Format"));
    s_formatMenu->addAction(s_boldAct);
    s_formatMenu->addAction(s_italicAct);
    s_formatMenu->addSeparator()->setText(tr("Alignment"));
    s_formatMenu->addAction(s_leftAlignAct);
    s_formatMenu->addAction(s_rightAlignAct);
    s_formatMenu->addAction(s_justifyAct);
    s_formatMenu->addAction(s_centerAct);
    s_formatMenu->addSeparator();
    s_formatMenu->addAction(s_setLineSpacingAct);
    s_formatMenu->addAction(s_setParagraphSpacingAct);
}

void MenuManager::onNewFile()
{
    // ...
}

void MenuManager::onOpen()
{
    // ...
}

void MenuManager::onSave()
{
    // ...
}

void MenuManager::onPrint()
{
    // ...
}

void MenuManager::onUndo()
{
    // ...
}

void MenuManager::onRedo()
{
    // ...
}

void MenuManager::onCut()
{
    // ...
}

void MenuManager::onCopy()
{
    // ...
}

void MenuManager::onPaste()
{
    // ...
}

void MenuManager::onBold()
{
    // ...
}

void MenuManager::onItalic()
{
    // ...
}

void MenuManager::onLeftAlign()
{
    // ...
}

void MenuManager::onRightAlign()
{
    // ...
}

void MenuManager::onJustify()
{
    // ...
}

void MenuManager::onCenter()
{
    // ...
}

void MenuManager::onSetLineSpacing()
{
    // ...
}

void MenuManager::onSetParagraphSpacing()
{
    // ...
}

void MenuManager::onPreferences()
{
    WindowManager::preferencesWindow()->show();
}

void MenuManager::onAbout()
{
    WindowManager::aboutWindow()->show();
}
