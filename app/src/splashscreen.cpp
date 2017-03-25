#include <splashscreen.h>
#include <splashscreen_p.h>

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

	void show()
	{
		d->show();
	}

	void hide()
	{
		d->hide();
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
		d->loadingImageFilename = loadingImageFilename;
	}

}
