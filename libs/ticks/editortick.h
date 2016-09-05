#ifndef EDITORTICK_H
#define EDITORTICK_H

#include <QPushButton>

class EditorTick : public QPushButton
{
		Q_OBJECT

	private:
		QWidget* m_TrackedWidget = nullptr;

	public:
		explicit EditorTick(QWidget *parent = 0);
		QWidget* TrackedWidget() const;
		void setTrackedWidget(QWidget* const);

	public slots:
		void fixCoord();

	private slots:
		void openEditor();

	protected:
		void paintEvent(QPaintEvent*);
		bool eventFilter(QObject* o, QEvent* e);

	signals:
		void editorOpened(QWidget* const);

};

#endif // EDITORTICK_H
