#ifndef QUICKITEM_H
#define QUICKITEM_H

#include <QQuickItem>

class QuickItem : public QQuickItem
{
		Q_OBJECT

	public:
		QuickItem();

	protected:
		QSGNode* updatePaintNode(QSGNode *node, UpdatePaintNodeData *);
};

#endif // QUICKITEM_H
