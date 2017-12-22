#ifndef ABOUTWIDGET_H
#define ABOUTWIDGET_H

#include <QWidget>

struct AboutWindowPrivate;

class AboutWindow : public QWidget
{
		Q_OBJECT
	public:
        explicit AboutWindow(QWidget *parent = 0);
        ~AboutWindow();

    protected:
        QSize sizeHint() const override;

	private:
		AboutWindowPrivate* _d;
};

#endif // ABOUTWIDGET_H
