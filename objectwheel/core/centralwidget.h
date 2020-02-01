#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QSplitter>
#include <modemanager.h>

class QmlCodeEditorWidget;
class ProjectOptionsWidget;
class HelpWidget;
class OutputPane;
class OutputController;
class BuildsPane;
class BuildsController;
class DesignerPane;
class DesignerController;
class EditorContainer;

class CentralWidget final : public QSplitter
{
    Q_OBJECT
    Q_DISABLE_COPY(CentralWidget)

public:
    explicit CentralWidget(QWidget* parent = nullptr);

    DesignerPane* designerPane() const;
    DesignerController* designerController() const;
    OutputPane* outputPane() const;
    OutputController* outputController() const;
    BuildsPane* buildsPane() const;
    BuildsController* buildsController() const;
    QmlCodeEditorWidget* qmlCodeEditorWidget() const;

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
    BuildsPane* m_buildsPane;
    BuildsController* m_buildsController;
    EditorContainer* m_codeEditorContainer;
    QmlCodeEditorWidget* m_qmlCodeEditorWidget;
    ProjectOptionsWidget* m_projectOptionsWidget;
    HelpWidget* m_helpWidget;
};

#endif // CENTRALWIDGET_H
