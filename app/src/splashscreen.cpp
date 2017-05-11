#include <splashscreen.h>
#include <splashscreen_p.h>
#include <QTimer>

namespace SplashScreen
{
	SplashScreenPrivate* d;

	void init(QWidget* parent)
	{
		d = new SplashScreenPrivate(parent);
	}

	void setIcon(const QIcon& icon)
	{
		d->icon = icon;
	}

	void setText(const QString& text)
	{
		d->text = text;
	}

	void setTextColor(const QColor& color)
	{
		d->textColor = color;
	}

	void setBackgroundBrush(const QBrush& brush)
	{
		d->backgroundBrush = brush;
	}

    void show(const bool animate, const int msec)
	{
        d->show(animate);
		if (msec > 0) QTimer::singleShot(msec, [=] { d->hide(); });
	}

	void hide(const int msec)
	{
		if (msec > 0) QTimer::singleShot(msec, [=] { d->hide(); });
		else d->hide();
	}

	void raise()
	{
		d->raise();
	}

	void setIconSize(const int w, const int h)
	{
		d->iconSize = QSize(w, h);
	}

	void setLoadingSize(const int w, const int h)
	{
		d->loadingSize = QSize(w, h);
	}

	void setLoadingImageFilename(const QString& loadingImageFilename)
	{
        d->setLoadingImage(loadingImageFilename);
	}
}
