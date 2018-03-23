#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QWidget>
#include <pages.h>

class QSplitter;
class QVBoxLayout;
class OutputPane;
class DesignerWidget;
class QmlCodeEditorWidget;
class ProjectOptionsWidget;
class HelpWidget;
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
        void setCurrentPage(const Pages& page);

    private slots:
        void hideWidgets();

    private:
        QVBoxLayout* m_layout;
        QSplitter* m_splitterOut,* m_splitterIn;
        OutputPane* m_outputPane;
        QmlCodeEditorWidget* m_qmlCodeEditorWidget;
        DesignerWidget* m_designerWidget;
        ProjectOptionsWidget* m_projectOptionsWidget;
        BuildsWidget* m_buildsWidget;
        HelpWidget* m_helpWidget;
};

#endif // CENTRALWIDGET_H
