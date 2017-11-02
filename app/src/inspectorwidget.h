#ifndef INSPECTORWIDGET_H
#define INSPECTORWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <filterlineedit.h>

class InspectorWidget : public QWidget
{
        Q_OBJECT
    public:
        explicit InspectorWidget(QWidget* parent = nullptr);

    public slots:
        void clear();
        void refresh();

    private slots:
        void handleDoubleClick(QTreeWidgetItem* item, int column); //Forward to designer manager
        void handleClick(QTreeWidgetItem* item, int column); //Forward to designer manager

    protected:
        virtual QSize sizeHint() const override;
        virtual bool eventFilter(QObject* watched, QEvent* event) override;

    private:
        QVBoxLayout _layout;
        QTreeWidget _treeWidget;
        bool _blockRefresh;
};

#endif // INSPECTORWIDGET_H
