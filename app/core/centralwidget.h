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
class OutputPane;
class OutputController;

class CentralWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CentralWidget(QWidget* parent = nullptr);
    QmlCodeEditorWidget* qmlCodeEditorWidget() const;
    DesignerWidget* designerWidget() const;
    OutputPane* outputPane() const;

public slots:
    void discharge();
    void setBottomPaneVisible(bool visible);

private slots:
    void hideWidgets();
    void onModeChange(ModeManager::Mode mode);

signals:
    bool bottomPaneTriggered(bool visible);

private:
    QVBoxLayout* m_layout;
    QSplitter* m_splitterOut,* m_splitterIn;
    OutputPane* m_outputPane;
    OutputController* m_outputController;
    QmlCodeEditorWidget* m_qmlCodeEditorWidget;
    DesignerWidget* m_designerWidget;
    ProjectOptionsWidget* m_projectOptionsWidget;
    BuildsWidget* m_buildsWidget;
    HelpWidget* m_helpWidget;
};

#endif // CENTRALWIDGET_H
