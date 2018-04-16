#ifndef MENUMANAGER_H
#define MENUMANAGER_H

#include <QObject>

class QMenuBar;
class QMenu;
class QAction;
class QActionGroup;

class MenuManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(MenuManager)

public:
    static MenuManager* instance();

public slots:
    inline void init() const {}

private:
    MenuManager();

private:
    void createMenus();
    void createActions();

private slots:
    void newFile();
    void open();
    void save();
    void print();
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void bold();
    void italic();
    void leftAlign();
    void rightAlign();
    void justify();
    void center();
    void setLineSpacing();
    void setParagraphSpacing();
    void preferences();
    void about();

private:
    QMenuBar* _menuBar;
    QMenu* fileMenu;
    QMenu* editMenu;
    QMenu* formatMenu;
    QMenu* helpMenu;
    QMenu* optionsMenu;
    QActionGroup* alignmentGroup;
    QAction* newAct;
    QAction* openAct;
    QAction* saveAct;
    QAction* printAct;
    QAction* exitAct;
    QAction* undoAct;
    QAction* redoAct;
    QAction* cutAct;
    QAction* copyAct;
    QAction* pasteAct;
    QAction* boldAct;
    QAction* italicAct;
    QAction* leftAlignAct;
    QAction* rightAlignAct;
    QAction* justifyAct;
    QAction* centerAct;
    QAction* setLineSpacingAct;
    QAction* setParagraphSpacingAct;
    QAction* preferencesAct;
    QAction* aboutAct;
    QAction* aboutQtAct;
};

#endif // MENUMANAGER_H
