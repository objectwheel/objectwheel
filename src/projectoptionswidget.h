#ifndef PROJECTOPTIONSWIDGET_H
#define PROJECTOPTIONSWIDGET_H

#include <QWidget>

class ProjectOptionsWidget : public QWidget
{
        Q_OBJECT
    public:
        explicit ProjectOptionsWidget(QWidget *parent = nullptr);

    protected:
        QSize sizeHint() const override;

    public slots:
        void reset();
};

#endif // PROJECTOPTIONSWIDGET_H