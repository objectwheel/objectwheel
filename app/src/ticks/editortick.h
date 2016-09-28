#ifndef EDITORTICK_H
#define EDITORTICK_H

#include <QPushButton>

class QQuickItem;

class EditorTick : public QPushButton
{
		Q_OBJECT

	private:
		QQuickItem* m_TrackedItem;

	public:
		explicit EditorTick(QWidget* const parent = 0);
		QQuickItem* TrackedItem() const;
		void SetTrackedItem(QQuickItem* const trackedItem);

	protected:
		void paintEvent(QPaintEvent* const event);
		bool eventFilter(QObject* object, QEvent* event);

	signals:
		void EditorOpened(QQuickItem* const item) const;

	public slots:
		void FixCoord();

	private slots:
		void OpenEditor();

};

#endif // EDITORTICK_H
