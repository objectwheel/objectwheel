#ifndef ABOUT_H
#define ABOUT_H

#include <QWidget>

struct AboutPrivate;

class About : public QWidget
{
		Q_OBJECT
	public:
		explicit About(QWidget* centralWidget, QWidget *parent = 0);
		~About();

	public slots:
		void show(bool);

	private:
		AboutPrivate* m_d;
};

#endif // ABOUT_H
