#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QWidget>
#include <modemanager.h>

class QSplitter;
class QVBoxLayout;
class DesignerWidget;
class QmlCodeEditorWidget;
class ProjectOptionsWidget;
class HelpWidget;
class BuildsWidget;
class BottomBar;
class ConsolePane;
class IssuesPane;

class CentralWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CentralWidget(QWidget* parent = nullptr);
    QmlCodeEditorWidget* qmlCodeEditorWidget() const;
    DesignerWidget* designerWidget() const;
    ConsolePane* consolePane() const;
    IssuesPane* issuesPane() const;
    BottomBar* bottomBar() const;

public slots:
    void discharge();
    
private slots:
    void hideWidgets();
    void onModeChange(ModeManager::Mode mode);

private:
    QVBoxLayout* m_layout;
    QSplitter* m_splitterOut,* m_splitterIn;
    BottomBar* m_bottomBar;
    ConsolePane* m_consolePane;
    IssuesPane* m_issuesPane;
    QmlCodeEditorWidget* m_qmlCodeEditorWidget;
    DesignerWidget* m_designerWidget;
    ProjectOptionsWidget* m_projectOptionsWidget;
    BuildsWidget* m_buildsWidget;
    HelpWidget* m_helpWidget;
};

#endif // CENTRALWIDGET_H
