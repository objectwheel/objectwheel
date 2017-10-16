#ifndef DESIGNMANAGER_H
#define DESIGNMANAGER_H

#include <formscene.h>
#include <qmleditorview.h>

#include <QObject>

class QWidget;
class DesignManagerPrivate;
class LoadingIndicator;

class DesignManager : public QObject
{
        Q_OBJECT
        Q_DISABLE_COPY(DesignManager)
        friend class DesignManagerPrivate;

    public:
        enum Mode {
            ControlGUI,
            FormGUI,
            CodeEdit
        };

    public:
        explicit DesignManager(QObject *parent = 0);
        static DesignManager* instance();
        static void setSettleWidget(QWidget* widget);
        static const Mode& mode();
        static void setMode(const Mode& mode);
        static ControlScene* currentScene();
        static ControlScene* controlScene();
        static FormScene* formScene();
        static QmlEditorView* qmlEditorView();
        static LoadingIndicator* loadingIndicator();

    public slots:
        void handleModeChange();

    signals:
        void modeChanged();

    private:
        static DesignManagerPrivate* _d;
        static Mode _mode;
        static ControlScene* _currentScene;
};

#endif // DESIGNMANAGER_H
