#include <listwidget.h>
#include <QMimeData>

ListWidget::ListWidget(QWidget *parent)
	: QListWidget(parent)
{
}

QMimeData* ListWidget::mimeData(const QList<QListWidgetItem*> items) const
{
	QMimeData *data = new QMimeData();
	data->setUrls(m_Urls[items[0]]);
	return data;
}
