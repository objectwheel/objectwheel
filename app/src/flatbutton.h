#ifndef FLATBUTTON_H
#define FLATBUTTON_H

#include <QPushButton>

class QGraphicsDropShadowEffect;

class FlatButton : public QPushButton
{
		Q_OBJECT

	private:
		QColor m_Color;
		QColor m_DisabledColor;
		QColor m_CheckedColor;
		QColor m_TextColor;
		QColor m_DisabledTextColor;
		QColor m_CheckedTextColor;
		QGraphicsDropShadowEffect* m_Shadow;
		int m_Radius;

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

	protected:
		void applyTheme();

	public slots:
};

#endif // FLATBUTTON_H
