#ifndef TOOLBOXTREE_H
#define TOOLBOXTREE_H

#include <QUrl>
#include <QTreeWidget>
#include <flatbutton.h>

class ToolboxTree : public QTreeWidget
{
		Q_OBJECT

	public:
        explicit ToolboxTree(QWidget *parent = 0);

        inline const QMap<QTreeWidgetItem*, QList<QUrl>>& allUrls() const { return _urls; }
        inline void addUrls(QTreeWidgetItem* item, const QList<QUrl>& urls) { _urls.insert(item, urls); }
        inline void removeUrls(QTreeWidgetItem* item) { _urls.remove(item); }
        inline QList<QUrl> urls(QTreeWidgetItem* item) const { return _urls.value(item); }
        inline void clearUrls() { _urls.clear(); }

        bool contains(const QString& itemName);
        bool categoryContains(const QString& categoryName);
        QTreeWidgetItem* categoryItem(const QString& categoryName);

        FlatButton* indicatorButton();
        void setIndicatorButtonVisible(bool value);

    protected:
        QMimeData* mimeData(const QList<QTreeWidgetItem*> items) const override;
        void mousePressEvent(QMouseEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;

    private:
        QMap<QTreeWidgetItem*, QList<QUrl>> _urls;
        FlatButton _indicatorButton;
        bool _indicatorButtonVisible;

    private:
        QPoint _previousPoint;
        QList<qreal> _angleList;
};

#endif // TOOLBOXTREE_H
