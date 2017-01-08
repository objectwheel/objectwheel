#ifndef SPLASHSCREEN_P_H
#define SPLASHSCREEN_P_H

#include <QWidget>
#include <QIcon>
#include <QColor>
#include <QString>
#include <QBrush>
#include <QMovie>
#include <QIODevice>

class SplashScreenPrivate : public QWidget
{
		Q_OBJECT
	public:
		QIcon icon;
		QSize iconSize;
		QSize loadingSize;
		QString text;
		QColor textColor;
		QBrush backgroundBrush;
		QMovie movie;
		explicit SplashScreenPrivate(QWidget *parent = 0);
		bool eventFilter(QObject *watched, QEvent *event) override;
		void setLoadingDevice(QIODevice* device);
		void paintEvent(QPaintEvent *event) override;
};

#endif // SPLASHSCREEN_P_H
