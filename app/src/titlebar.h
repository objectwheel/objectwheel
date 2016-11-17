#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>

class QLabel;
class QHBoxLayout;
class QVBoxLayout;
class QPushButton;
class QGraphicsDropShadowEffect;

class TitleBar : public QWidget
{
		Q_OBJECT
	private:
		static const uchar m_SettingsIconData[];
		static const uchar m_MenuIconData[];
		QVBoxLayout* m_Layout;
		QHBoxLayout* m_UpperLayout;
		QLabel* m_Title;
		QPushButton* m_Menu;
		QPushButton* m_Settings;
		QColor m_Color;
		QString m_Text;
		QWidget* m_ShadowWidgetBackground;
		QWidget* m_ShadowWidget;
		QColor m_ShadowWidgetColor;

	public:
		explicit TitleBar(QWidget *parent = 0);

		const QColor& Color() const;
		void setColor(const QColor& Color);

		const QString& Text() const;
		void setText(const QString& Text);

		const QColor& ShadowColor() const;
		void setShadowColor(const QColor& ShadowColor);

	signals:
		void MenuToggled(bool);
		void SettingsToggled(bool);

	public slots:
		void hideButtons();
		void showButtons();
		void setMenuChecked(bool checked);
		void setSettingsChecked(bool checked);
};

#endif // TITLEBAR_H
