#ifndef QMLEDITORVIEW_H
#define QMLEDITORVIEW_H

#include <QWidget>

class QmlEditorViewPrivate;

class QmlEditorView : public QWidget
{
        Q_OBJECT
    public:
        explicit QmlEditorView(QWidget *parent = 0);

    protected:
        virtual void paintEvent(QPaintEvent *event) override;

    private:
        QmlEditorViewPrivate* _d;
};

#endif // QMLEDITORVIEW_H
