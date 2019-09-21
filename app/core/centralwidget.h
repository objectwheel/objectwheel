#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QSplitter>
#include <modemanager.h>

class QmlCodeEditorWidget;
class ProjectOptionsWidget;
class HelpWidget;
class BuildsWidget;
class OutputPane;
class OutputController;
class DesignerPane;
class DesignerController;

class CentralWidget final : public QSplitter
{
    Q_OBJECT
    Q_DISABLE_COPY(CentralWidget)

public:
    explicit CentralWidget(QWidget* parent = nullptr);
    QmlCodeEditorWidget* qmlCodeEditorWidget() const;
    OutputPane* outputPane() const;
    OutputController* outputController() const;
    DesignerPane* designerPane() const;
    DesignerController* designerController() const;

public slots:
    void charge();
    void discharge();

private slots:
    void hideWidgets();
    void onModeChange(ModeManager::Mode mode);

private:
    QSplitter* m_splitterIn;
    OutputPane* m_outputPane;
    OutputController* m_outputController;
    DesignerPane* m_designerPane;
    DesignerController* m_designerController;
    QmlCodeEditorWidget* m_qmlCodeEditorWidget;
    ProjectOptionsWidget* m_projectOptionsWidget;
    BuildsWidget* m_buildsWidget;
    HelpWidget* m_helpWidget;
};

#endif // CENTRALWIDGET_H
