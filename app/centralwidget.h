#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QWidget>
#include <pages.h>

class QSplitter;
class QVBoxLayout;
class DesignerWidget;
class QmlCodeEditorWidget;
class ProjectOptionsWidget;
class HelpWidget;
class BuildsWidget;
class BottomBar;
class ConsoleBox;
class IssuesBox;

class CentralWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CentralWidget(QWidget* parent = nullptr);
    QmlCodeEditorWidget* qmlCodeEditorWidget() const;
    DesignerWidget* designerWidget() const;
    ConsoleBox* consoleBox() const;
    IssuesBox* issuesBox() const;

public slots:
    void sweep();
    void setCurrentPage(const Pages& page);

private slots:
    void hideWidgets();

private:
    QVBoxLayout* m_layout;
    QSplitter* m_splitterOut,* m_splitterIn;
    BottomBar* m_bottomBar;
    ConsoleBox* m_consoleBox;
    IssuesBox* m_issuesBox;
    QmlCodeEditorWidget* m_qmlCodeEditorWidget;
    DesignerWidget* m_designerWidget;
    ProjectOptionsWidget* m_projectOptionsWidget;
    BuildsWidget* m_buildsWidget;
    HelpWidget* m_helpWidget;
};

#endif // CENTRALWIDGET_H
