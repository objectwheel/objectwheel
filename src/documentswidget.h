#ifndef DOCUMENTSWIDGET_H
#define DOCUMENTSWIDGET_H

#include <QWidget>

class DocumentsWidget : public QWidget
{
        Q_OBJECT
    public:
        explicit DocumentsWidget(QWidget *parent = nullptr);

    protected:
        QSize sizeHint() const override;

    public slots:
        void reset();
};

#endif // DOCUMENTSWIDGET_H