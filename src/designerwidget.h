#ifndef DESIGNERWIDGET_H
#define DESIGNERWIDGET_H

#include <formscene.h>
#include <qmleditorview.h>
#include <global.h>
#include <QObject>

class QWidget;
class DesignerWidgetPrivate;
class LoadingIndicator;
class OutputWidget;
class QSplitter;
class ControlView;
class FormView;

class DesignerWidget : public QObject
{
        Q_OBJECT
        Q_DISABLE_COPY(DesignerWidget)
        friend class DesignerWidgetPrivate;

    public:
        explicit DesignerWidget(QObject *parent = 0);
        static DesignerWidget* instance();
        static void setSettleWidget(QWidget* widget);
        static const DesignMode& mode();
        static void setMode(const DesignMode& mode);
        static ControlScene* currentScene();
        static ControlScene* controlScene();
        static FormScene* formScene();
        static QmlEditorView* qmlEditorView();
        static ControlView* controlView();
        static FormView* formView();
        static LoadingIndicator* loadingIndicator();
        static QSplitter* splitter();
        static OutputWidget* outputWidget();

    public slots:
        void updateSkin();
        void checkErrors();
        void controlClicked(Control*);
        void controlDoubleClicked(Control*);
        void controlDropped(Control*, const QPointF&, const QString&);

    signals:
        void modeChanged();

    private:
        static DesignerWidgetPrivate* _d;
        static DesignMode _mode;
        static ControlScene* _currentScene;
};

#endif // DESIGNERWIDGET_H
