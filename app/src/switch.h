#ifndef SWITCH_H
#define SWITCH_H

#include <QAbstractButton>

class QTimer;


class Switch : public QAbstractButton
{
		Q_OBJECT

	private:
		int m_x;
		int m_indicatorWidth;
		QTimer* m_shifterTimer;
		QColor m_indicatorColor;
		QColor m_activeColor;
		QColor m_passiveColor;

	public:
		explicit Switch(QWidget *parent = 0);
		virtual QSize sizeHint() const override;
		virtual QSize minimumSizeHint() const override;

		virtual void setIndicatorColor(const QColor& color);
		virtual void setActiveColor(const QColor& color);
		virtual void setPassiveColor(const QColor& color);
		virtual const QColor& indicatorColor() const;
		virtual const QColor& activeColor() const;
		virtual const QColor& passiveColor() const;

	protected:
		virtual void paintEvent(QPaintEvent *e) override;
		virtual void resizeEvent(QResizeEvent *e) override;

	private slots:
		void shiftIndicator();
};

#endif // SWITCH_H
