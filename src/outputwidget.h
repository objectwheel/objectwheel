#ifndef OUTPUTWIDGET_H
#define OUTPUTWIDGET_H

#include <QWidget>

class OutputWidget : public QWidget
{
        Q_OBJECT
    public:
        explicit OutputWidget(QWidget *parent = nullptr);

    public slots:
        void reset();
};

#endif // OUTPUTWIDGET_H