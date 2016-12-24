#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QWidget>

class LineEditPrivate;

class LineEdit : public QWidget
{
		Q_OBJECT

	public:
		explicit LineEdit(QWidget* parent = 0);
		void setIcon(const QIcon& icon);
		~LineEdit();

	private:
		LineEditPrivate* m_d;
};





#endif // LINEEDIT_H
