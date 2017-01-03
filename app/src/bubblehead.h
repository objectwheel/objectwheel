#ifndef BUBBLEHEAD_H
#define BUBBLEHEAD_H

#include <QPushButton>

class BubbleHeadPrivate;

class BubbleHead : public QPushButton
{
		Q_OBJECT

	public:
		explicit BubbleHead(QWidget *parent = 0);
		~BubbleHead();

		void setIcon(const QIcon& icon);
		const QIcon& icon() const;

		void setBorderColor(const QColor& color);
		const QColor& borderColor() const;

		void setNotificationText(const QString& text);
		const QString& notificationText() const;

	protected:
		void mousePressEvent(QMouseEvent *event) override;
		void mouseMoveEvent(QMouseEvent *event) override;
		void mouseReleaseEvent(QMouseEvent *event) override;
		void resizeEvent(QResizeEvent *event) override;
		void paintEvent(QPaintEvent *event) override;

	private:
		BubbleHeadPrivate* m_d;
};

#endif // BUBBLEHEAD_H
