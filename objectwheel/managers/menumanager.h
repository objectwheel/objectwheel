#ifndef MENUMANAGER_H
#define MENUMANAGER_H

#include <QObject>

class QAction;
class QActionGroup;
class QMenu;
class QMenuBar;

// TODO: Complete this class's functionality
class MenuManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(MenuManager)

    friend class ApplicationCore;

private:
    static void onNewFile();
    static void onOpen();
    static void onSave();
    static void onPrint();
    static void onUndo();
    static void onRedo();
    static void onCut();
    static void onCopy();
    static void onPaste();
    static void onBold();
    static void onItalic();
    static void onLeftAlign();
    static void onRightAlign();
    static void onJustify();
    static void onCenter();
    static void onSetLineSpacing();
    static void onSetParagraphSpacing();
    static void onPreferences();
    static void onAbout();

    static void createMenus();
    static void createActions(QObject* parent);

private:
    explicit MenuManager(QObject* parent = nullptr);

private:
    static QMenuBar* s_menuBar;
    static QMenu* s_fileMenu;
    static QMenu* s_editMenu;
    static QMenu* s_formatMenu;
    static QMenu* s_helpMenu;
    static QMenu* s_optionsMenu;
    static QActionGroup* s_alignmentGroup;
    static QAction* s_newAct;
    static QAction* s_openAct;
    static QAction* s_saveAct;
    static QAction* s_printAct;
    static QAction* s_exitAct;
    static QAction* s_undoAct;
    static QAction* s_redoAct;
    static QAction* s_cutAct;
    static QAction* s_copyAct;
    static QAction* s_pasteAct;
    static QAction* s_boldAct;
    static QAction* s_italicAct;
    static QAction* s_leftAlignAct;
    static QAction* s_rightAlignAct;
    static QAction* s_justifyAct;
    static QAction* s_centerAct;
    static QAction* s_setLineSpacingAct;
    static QAction* s_setParagraphSpacingAct;
    static QAction* s_preferencesAct;
    static QAction* s_aboutAct;
};

#endif // MENUMANAGER_H
