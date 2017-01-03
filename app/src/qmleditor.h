#ifndef QMLEDITOR_H
#define QMLEDITOR_H

#include <QWidget>

class QmlEditorPrivate;

class QmlEditor : public QWidget
{
		Q_OBJECT

	public:
		explicit QmlEditor(QWidget *parent = 0);
		~QmlEditor();

	private:
		QmlEditorPrivate* m_d;
};

#endif // QMLEDITOR_H
