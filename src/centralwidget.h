#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QWidget>

class QSplitter;
class QVBoxLayout;
class OutputPane;
class DesignerWidget;
class QmlCodeEditorWidget;
class ProjectSettingsWidget;
class DocumentationsWidget;
class BuildsWidget;

class CentralWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit CentralWidget(QWidget* parent = nullptr);
        QmlCodeEditorWidget* qmlCodeEditorWidget() const;
        DesignerWidget* designerWidget() const;
        OutputPane* outputPane() const;

    public slots:
        void reset();

    private:
        QVBoxLayout* m_layout;
        QSplitter* m_splitterOut,* m_splitterIn;
        OutputPane* m_outputPane;
        QmlCodeEditorWidget* m_qmlCodeEditorWidget;
        DesignerWidget* m_designerWidget;
        ProjectSettingsWidget* m_projectSettingsWidget;
        BuildsWidget* m_buildsWidget;
        DocumentationsWidget* m_documentationsWidget;
};

#endif // CENTRALWIDGET_H
