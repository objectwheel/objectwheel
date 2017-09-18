#ifndef QMLTEXTEDITOR_H
#define QMLTEXTEDITOR_H

#include <QTextEdit>

class QmlTextEditor : public QTextEdit
{
        Q_OBJECT
    public:
        explicit QmlTextEditor(QWidget *parent = 0);

    signals:

    public slots:
};

#endif // QMLTEXTEDITOR_H
