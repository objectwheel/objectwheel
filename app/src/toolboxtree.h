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

        void addUrls(QTreeWidgetItem* item, const QList<QUrl>& urls);
        void removeUrls(QTreeWidgetItem* item);
        void clearUrls();

        const QMap<QTreeWidgetItem*, QList<QUrl>>& allUrls() const;
        QList<QUrl> urls(QTreeWidgetItem* item) const;

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
