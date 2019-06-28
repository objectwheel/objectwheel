#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QSplitter>
#include <modemanager.h>

class DesignerWidget;
class QmlCodeEditorWidget;
class ProjectOptionsWidget;
class HelpWidget;
class BuildsWidget;
class OutputPane;
class OutputController;

class CentralWidget final : public QSplitter
{
    Q_OBJECT
    Q_DISABLE_COPY(CentralWidget)

public:
    explicit CentralWidget(QWidget* parent = nullptr);
    QmlCodeEditorWidget* qmlCodeEditorWidget() const;
    DesignerWidget* designerWidget() const;
    OutputPane* outputPane() const;

public slots:
    void discharge();

private slots:
    void hideWidgets();
    void onModeChange(ModeManager::Mode mode);

signals:
    bool bottomPaneTriggered(bool visible);

private:
    QSplitter* m_splitterIn;
    OutputPane* m_outputPane;
    OutputController* m_outputController;
    QmlCodeEditorWidget* m_qmlCodeEditorWidget;
    DesignerWidget* m_designerWidget;
    ProjectOptionsWidget* m_projectOptionsWidget;
    BuildsWidget* m_buildsWidget;
    HelpWidget* m_helpWidget;
};

#endif // CENTRALWIDGET_H
