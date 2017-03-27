#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

class QIcon;
class QColor;
class QString;
class QWidget;
class QSize;
class QBrush;
class QIODevice;

namespace SplashScreen
{
	void init(QWidget* parent);
	void setIcon(const QIcon& icon);
	void setIconSize(const int w, const int h);
	void setLoadingSize(const int w, const int h);
	void setText(const QString& text);
	void setTextColor(const QColor& color);
	void setBackgroundBrush(const QBrush& brush);
	void setLoadingImageFilename(const QString& loadingImageFilename);
	void raise();
	void show();
	void hide();
}

#endif // SPLASHSCREEN_H
