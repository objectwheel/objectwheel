#ifndef TREEWIDGET_H
#define TREEWIDGET_H

#include <QUrl>
#include <QListWidget>

class ListWidget : public QListWidget
{
		Q_OBJECT

	private:
		QMap<QListWidgetItem*, QList<QUrl>> m_Urls;
		QPoint m_PreviousPoint;
		QList<qreal> m_AngleList;
		QTimer* m_HiderTimer;
		QTimer* m_DelayTimer;
		int m_Alpha;
		bool m_Running;

	public:
		explicit ListWidget(QWidget *parent = 0);
		inline const QMap<QListWidgetItem*, QList<QUrl>>& GetUrls() const { return m_Urls; }
		inline void AddUrls(QListWidgetItem* item, const QList<QUrl>& urls) { m_Urls.insert(item, urls); }
		inline void RemoveUrls(QListWidgetItem* item) { m_Urls.remove(item); }
		inline QList<QUrl> GetUrls(QListWidgetItem* item) const { return m_Urls.value(item); }
		inline void ClearUrls() { m_Urls.clear(); }

	protected:
		QMimeData* mimeData(const QList<QListWidgetItem *> items) const;
		void mousePressEvent(QMouseEvent* const event);
		void mouseMoveEvent(QMouseEvent* const event);
		void wheelEvent(QWheelEvent* const event);
		void setBarOpacity(const int opacity);
		int getBarOpacity() const;

	public slots:
		void showBar();
		void hideBar();
};

#endif // TREEWIDGET_H
