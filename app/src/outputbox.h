#ifndef OUTPUTBOX_H
#define OUTPUTBOX_H

#include <global.h>
#include <QWidget>
#include <QMap>
#include <QPointer>

class QSplitter;
class QSplitterHandle;
class OutputBoxPrivate;

enum BoxType {
    Issues,
    Search,
    Console
};

class OutputBox : public QWidget
{
        Q_OBJECT
        friend class OutputBoxPrivate;

    public:
        explicit OutputBox(QWidget *parent = nullptr);
        void setSplitter(QSplitter* splitter);
        void setSplitterHandle(QSplitterHandle* splitterHandle);
        void setActiveBox(BoxType type);
        BoxType activeBoxType() const;
        bool collapsed() const;

    public slots:
        void expand();
        void collapse();
        void updateLastHeight();

    protected:
        virtual QSize sizeHint() const override;

    private:
        QPointer<QSplitter> _splitter;
        QPointer<QSplitterHandle> _splitterHandle;
        OutputBoxPrivate* _d;
        int _lastHeight;
        BoxType _activeBoxType;
        bool _collapsed;
};

#endif // OUTPUTBOX_H
