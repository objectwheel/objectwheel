#ifndef DOCUMENTATIONSWIDGET_H
#define DOCUMENTATIONSWIDGET_H

#include <QWidget>

class DocumentationsWidget : public QWidget
{
        Q_OBJECT
    public:
        explicit DocumentationsWidget(QWidget *parent = nullptr);

    protected:
        QSize sizeHint() const override;

    public slots:
        void reset();
};

#endif // DOCUMENTATIONSWIDGET_H