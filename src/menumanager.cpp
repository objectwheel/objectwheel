#include <menumanager.h>
#include <windowmanager.h>
#include <QMenuBar>
#include <QMainWindow>
#include <QApplication>

MenuManager::MenuManager()
{
    #if defined(Q_OS_MAC)
    _menuBar = new QMenuBar;
    #else
    _menuBar = ((QMainWindow*)WindowManager::instance()->get(WindowManager::Main))->menuBar();
    _menuBar->setStyleSheet("QMenuBar {"
                            "    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
                            "    stop:0 #ffffff, stop:1 #d6d9dc);"
                            "}");
    QFont f;
    f.setPixelSize(f.pixelSize() - 1);
    _menuBar->setFont(f);
    #endif
    createActions();
    createMenus();
}

MenuManager::~MenuManager()
{
    delete _menuBar;
}

MenuManager* MenuManager::instance()
{
    static MenuManager instance;
    return &instance;
}

void MenuManager::createActions()
{
    newAct = new QAction(tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, &QAction::triggered, this, &MenuManager::newFile);

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, &MenuManager::open);

    saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, &QAction::triggered, this, &MenuManager::save);

    printAct = new QAction(tr("&Print..."), this);
    printAct->setShortcuts(QKeySequence::Print);
    printAct->setStatusTip(tr("Print the document"));
    connect(printAct, &QAction::triggered, this, &MenuManager::print);

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    // TODO: connect(exitAct, &QAction::triggered, this, &QWidget::close);

    undoAct = new QAction(tr("&Undo"), this);
    undoAct->setShortcuts(QKeySequence::Undo);
    undoAct->setStatusTip(tr("Undo the last operation"));
    connect(undoAct, &QAction::triggered, this, &MenuManager::undo);

    redoAct = new QAction(tr("&Redo"), this);
    redoAct->setShortcuts(QKeySequence::Redo);
    redoAct->setStatusTip(tr("Redo the last operation"));
    connect(redoAct, &QAction::triggered, this, &MenuManager::redo);

    cutAct = new QAction(tr("Cu&t"), this);
    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cutAct, &QAction::triggered, this, &MenuManager::cut);

    copyAct = new QAction(tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, &QAction::triggered, this, &MenuManager::copy);

    pasteAct = new QAction(tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(pasteAct, &QAction::triggered, this, &MenuManager::paste);

    boldAct = new QAction(tr("&Bold"), this);
    boldAct->setCheckable(true);
    boldAct->setShortcut(QKeySequence::Bold);
    boldAct->setStatusTip(tr("Make the text bold"));
    connect(boldAct, &QAction::triggered, this, &MenuManager::bold);

    QFont boldFont = boldAct->font();
    boldFont.setBold(true);
    boldAct->setFont(boldFont);

    italicAct = new QAction(tr("&Italic"), this);
    italicAct->setCheckable(true);
    italicAct->setShortcut(QKeySequence::Italic);
    italicAct->setStatusTip(tr("Make the text italic"));
    connect(italicAct, &QAction::triggered, this, &MenuManager::italic);

    QFont italicFont = italicAct->font();
    italicFont.setItalic(true);
    italicAct->setFont(italicFont);

    setLineSpacingAct = new QAction(tr("Set &Line Spacing..."), this);
    setLineSpacingAct->setStatusTip(tr("Change the gap between the lines of a "
                                       "paragraph"));
    connect(setLineSpacingAct, &QAction::triggered, this, &MenuManager::setLineSpacing);

    setParagraphSpacingAct = new QAction(tr("Set &Paragraph Spacing..."), this);
    setParagraphSpacingAct->setStatusTip(tr("Change the gap between paragraphs"));
    connect(setParagraphSpacingAct, &QAction::triggered,
            this, &MenuManager::setParagraphSpacing);

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, &QAction::triggered, this, &MenuManager::about);

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, &QAction::triggered, qApp, &QApplication::aboutQt);

    preferencesAct = new QAction(tr("&Preferences"), this);
    preferencesAct->setStatusTip(tr("Show Objectwheel Preferences"));
    connect(preferencesAct, &QAction::triggered, this, &MenuManager::preferences);

    leftAlignAct = new QAction(tr("&Left Align"), this);
    leftAlignAct->setCheckable(true);
    leftAlignAct->setShortcut(tr("Ctrl+L"));
    leftAlignAct->setStatusTip(tr("Left align the selected text"));
    connect(leftAlignAct, &QAction::triggered, this, &MenuManager::leftAlign);

    rightAlignAct = new QAction(tr("&Right Align"), this);
    rightAlignAct->setCheckable(true);
    rightAlignAct->setShortcut(tr("Ctrl+R"));
    rightAlignAct->setStatusTip(tr("Right align the selected text"));
    connect(rightAlignAct, &QAction::triggered, this, &MenuManager::rightAlign);

    justifyAct = new QAction(tr("&Justify"), this);
    justifyAct->setCheckable(true);
    justifyAct->setShortcut(tr("Ctrl+J"));
    justifyAct->setStatusTip(tr("Justify the selected text"));
    connect(justifyAct, &QAction::triggered, this, &MenuManager::justify);

    centerAct = new QAction(tr("&Center"), this);
    centerAct->setCheckable(true);
    centerAct->setShortcut(tr("Ctrl+E"));
    centerAct->setStatusTip(tr("Center the selected text"));
    connect(centerAct, &QAction::triggered, this, &MenuManager::center);

    alignmentGroup = new QActionGroup(this);
    alignmentGroup->addAction(leftAlignAct);
    alignmentGroup->addAction(rightAlignAct);
    alignmentGroup->addAction(justifyAct);
    alignmentGroup->addAction(centerAct);
    leftAlignAct->setChecked(true);
}

void MenuManager::createMenus()
{
    fileMenu = _menuBar->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(printAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    editMenu = _menuBar->addMenu(tr("&Edit"));
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addSeparator();
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);
    editMenu->addSeparator();

    optionsMenu = _menuBar->addMenu(tr("&Options"));

    helpMenu = _menuBar->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);


#if defined (Q_OS_DARWIN)
    helpMenu->addSeparator();
    helpMenu->addAction(preferencesAct);
#else
    optionsMenu->addAction(preferencesAct);
#endif

    formatMenu = editMenu->addMenu(tr("&Format"));
    formatMenu->addAction(boldAct);
    formatMenu->addAction(italicAct);
    formatMenu->addSeparator()->setText(tr("Alignment"));
    formatMenu->addAction(leftAlignAct);
    formatMenu->addAction(rightAlignAct);
    formatMenu->addAction(justifyAct);
    formatMenu->addAction(centerAct);
    formatMenu->addSeparator();
    formatMenu->addAction(setLineSpacingAct);
    formatMenu->addAction(setParagraphSpacingAct);
}


void MenuManager::newFile()
{
    //TODO
}

void MenuManager::open()
{
    //TODO
}

void MenuManager::save()
{
    //TODO
}

void MenuManager::print()
{
    //TODO
}

void MenuManager::undo()
{
    //TODO
}

void MenuManager::redo()
{
    //TODO
}

void MenuManager::cut()
{
    //TODO
}

void MenuManager::copy()
{
    //TODO
}

void MenuManager::paste()
{
    //TODO
}

void MenuManager::bold()
{
    //TODO
}

void MenuManager::italic()
{
    //TODO
}

void MenuManager::leftAlign()
{
    //TODO
}

void MenuManager::rightAlign()
{
    //TODO
}

void MenuManager::justify()
{
    //TODO
}

void MenuManager::center()
{
    //TODO
}

void MenuManager::setLineSpacing()
{
    //TODO
}

void MenuManager::setParagraphSpacing()
{
    //TODO
}

void MenuManager::preferences()
{
    WindowManager::instance()->show(WindowManager::Preferences);
}

void MenuManager::about()
{
    WindowManager::instance()->show(WindowManager::About);
}
