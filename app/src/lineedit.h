#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QWidget>

class LineEditPrivate;
class QLineEdit;

class LineEdit : public QWidget
{
		Q_OBJECT

	public:
		explicit LineEdit(QWidget* parent = 0);
		void setIcon(const QIcon& icon);
		void setPlaceholderText(const QString& text);
		void setColor(const QColor& color);
		void setText(const QString& text);
		const QString text() const;
		QLineEdit* lineEdit();
		~LineEdit();

	private:
		LineEditPrivate* m_d;
};





#endif // LINEEDIT_H
