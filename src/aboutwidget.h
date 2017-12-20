#ifndef ABOUTWIDGET_H
#define ABOUTWIDGET_H

#include <QWidget>

struct AboutWidgetPrivate;

class AboutWidget : public QWidget
{
		Q_OBJECT
	public:
        explicit AboutWidget(QWidget *parent = 0);
        ~AboutWidget();

    protected:
        QSize sizeHint() const override;

	private:
		AboutWidgetPrivate* _d;
};

#endif // ABOUTWIDGET_H
