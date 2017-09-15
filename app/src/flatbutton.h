#ifndef FLATBUTTON_H
#define FLATBUTTON_H

#include <QPushButton>

class QGraphicsDropShadowEffect;

class FlatButton : public QPushButton
{
		Q_OBJECT

	private:
		QColor m_Color;
		QColor _disabledColor;
		QColor m_CheckedColor;
		QColor m_TextColor;
		QColor _disabledTextColor;
		QColor m_CheckedTextColor;
		int m_Radius;
		bool m_IconButton;
		bool _down;

	public:
		explicit FlatButton(QWidget *parent = 0);

		const QColor& Color() const;
		void setColor(const QColor& Color);
		const QColor& DisabledColor() const;
		void setDisabledColor(const QColor& Color);
		const QColor& CheckedColor() const;
		void setCheckedColor(const QColor& Color);

		const QColor& TextColor() const;
		void setTextColor(const QColor& TextColor);
		const QColor& DisabledTextColor() const;
		void setDisabledTextColor(const QColor& TextColor);
		const QColor& CheckedTextColor() const;
		void setCheckedTextColor(const QColor& TextColor);

		int radius() const;
		void setRadius(int value);

		bool IconButton() const;
		void setIconButton(bool IconButton);

	protected:
		virtual void applyTheme();
		virtual void paintEvent(QPaintEvent *);
};

#endif // FLATBUTTON_H
