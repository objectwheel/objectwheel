#ifndef SPLASHSCREEN_P_H
#define SPLASHSCREEN_P_H

#include <QWidget>
#include <QIcon>
#include <QColor>
#include <QString>
#include <QBrush>
#include <QMovie>
#include <QIODevice>
#include <QQuickWidget>
#include <QTimer>

class SplashScreenPrivate : public QWidget
{
		Q_OBJECT
		Q_PROPERTY(float showRatio READ getShowRatio WRITE setShowRatio)

	public:
		QIcon icon;
		QSize iconSize;
		QSize loadingSize;
		QString text;
		QColor textColor;
		QBrush backgroundBrush;
		QQuickWidget loadingWidget;
		QString loadingImageFilename;
		QQuickItem* prevBusyIndicator;
		QTimer waitEffectTimer;
		QString waitEffectString;
		float showRatio;

		explicit SplashScreenPrivate(QWidget *parent = 0);
		bool eventFilter(QObject *watched, QEvent *event) override;
		void paintEvent(QPaintEvent *event) override;
		float getShowRatio() const;
		void setShowRatio(float value);
		void hide();
		void show();
};

#endif // SPLASHSCREEN_P_H
