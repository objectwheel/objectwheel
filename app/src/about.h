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

    protected:
        void resizeEvent(QResizeEvent *event) override;

    signals:
        void resized() const;

	private:
		AboutPrivate* m_d;
};

#endif // ABOUT_H
