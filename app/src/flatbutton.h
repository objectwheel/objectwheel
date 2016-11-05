#ifndef FLATBUTTON_H
#define FLATBUTTON_H

#include <QPushButton>

class QGraphicsDropShadowEffect;

class FlatButton : public QPushButton
{
		Q_OBJECT

	private:
		QColor m_Color;
		QColor m_TextColor;
		QGraphicsDropShadowEffect* m_Shadow;

	public:
		explicit FlatButton(QWidget *parent = 0);

		const QColor& Color() const;
		void setColor(const QColor& Color);

		const QColor& TextColor() const;
		void setTextColor(const QColor& TextColor);

	protected:
		void applyTheme();

	public slots:
};

#endif // FLATBUTTON_H
