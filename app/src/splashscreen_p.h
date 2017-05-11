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
#include <QGraphicsOpacityEffect>

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
        QQuickItem* busyIndicator;
		QTimer waitEffectTimer;
		QString waitEffectString;
		float showRatio;
        QTimer showerTimer;
        QGraphicsOpacityEffect opacityEffect;

		explicit SplashScreenPrivate(QWidget *parent = 0);
		bool eventFilter(QObject *watched, QEvent *event) override;
		void paintEvent(QPaintEvent *event) override;
        void setLoadingImage(const QString& filename);
		float getShowRatio() const;
		void setShowRatio(float value);
		void hide();
        void show(const bool animated);
};

#endif // SPLASHSCREEN_P_H
