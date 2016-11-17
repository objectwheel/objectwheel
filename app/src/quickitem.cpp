#include <quickitem.h>
#include <QSGSimpleRectNode>

QuickItem::QuickItem()
{

}

QSGNode* QuickItem::updatePaintNode(QSGNode* node, QQuickItem::UpdatePaintNodeData*)
{
	QSGSimpleRectNode *n = static_cast<QSGSimpleRectNode *>(node);
	if (!n) {
		n = new QSGSimpleRectNode();
		n->setColor(Qt::red);
	}
	n->setRect(boundingRect());
	return n;
}
