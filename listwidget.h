#ifndef TREEWIDGET_H
#define TREEWIDGET_H

#include <QUrl>
#include <QListWidget>

class ListWidget : public QListWidget
{
		Q_OBJECT

	private:
		QMap<QListWidgetItem*, QList<QUrl>> m_Urls;

	public:
		explicit ListWidget(QWidget *parent = 0);
		inline const QMap<QListWidgetItem*, QList<QUrl>>& GetUrls() const { return m_Urls; }
		inline void AddUrls(QListWidgetItem* item, const QList<QUrl>& urls) { m_Urls.insert(item, urls); }
		inline void RemoveUrls(QListWidgetItem* item) { m_Urls.remove(item); }

	protected:
		QMimeData* mimeData(const QList<QListWidgetItem *> items) const;
};

#endif // TREEWIDGET_H
