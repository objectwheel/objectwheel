#ifndef ABOUT_H
#define ABOUT_H

#include <QWidget>

struct AboutPrivate;

class About : public QWidget
{
		Q_OBJECT
	public:
		explicit About(QWidget *parent = 0);
		~About();

	private:
		AboutPrivate* m_d;
};

#endif // ABOUT_H
