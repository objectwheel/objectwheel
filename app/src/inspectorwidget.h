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
        explicit InspectorWidget(QWidget *parent = nullptr);

    public slots:
        void clearList();
        void refreshList();

    protected:
        virtual QSize sizeHint() const override;

    private:
        QVBoxLayout _layout;
        QTreeWidget _treeWidget;
};

#endif // INSPECTORWIDGET_H
