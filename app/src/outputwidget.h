#ifndef OUTPUTWIDGET_H
#define OUTPUTWIDGET_H

#include <global.h>
#include <QWidget>
#include <QMap>
#include <QPointer>
#include <QTimer>

class QSplitter;
class QSplitterHandle;
class OutputWidgetPrivate;

enum BoxType {
    Issues,
    Search,
    Console
};

class OutputWidget : public QWidget
{
        Q_OBJECT
        friend class OutputWidgetPrivate;

    public:
        explicit OutputWidget(QWidget *parent = nullptr);
        void setSplitter(QSplitter* splitter);
        void setSplitterHandle(QSplitterHandle* splitterHandle);
        void setActiveBox(BoxType type);
        BoxType activeBoxType() const;
        bool collapsed() const;

    public slots:
        void expand();
        void collapse();
        void updateLastHeight();
        void shine(BoxType type);

    protected:
        virtual QSize sizeHint() const override;

    private:
        QPointer<QSplitter> _splitter;
        QPointer<QSplitterHandle> _splitterHandle;
        OutputWidgetPrivate* _d;
        int _lastHeight;
        BoxType _activeBoxType;
        bool _collapsed;
};

#endif // OUTPUTWIDGET_H
