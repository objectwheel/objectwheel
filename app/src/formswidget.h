#ifndef FORMSWIDGET_H
#define FORMSWIDGET_H

#include <QWidget>

class FormsWidgetPrivate;

class FormsWidget : public QWidget
{
        Q_OBJECT
        Q_DISABLE_COPY(FormsWidget)

    public:
        explicit FormsWidget(QWidget *parent = 0);
        static FormsWidget* instance();
        static void setCurrentForm(int index);

    protected:
        virtual QSize sizeHint() const override;

    private:
        static FormsWidgetPrivate* _d;
};

#endif // FORMSWIDGET_H
