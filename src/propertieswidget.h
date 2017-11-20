#ifndef PROPERTIESWIDGET_H
#define PROPERTIESWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <filterlineedit.h>

class PropertiesWidget : public QWidget
{
		Q_OBJECT

	public:
		explicit PropertiesWidget(QWidget *parent = 0);

	public slots:
        void clearList();
        void refreshList();
        void handleSelectionChange();

    private slots:
        void filterList(const QString& filter);

    protected:
        virtual QSize sizeHint() const override;
        virtual bool eventFilter(QObject *watched, QEvent *event) override;

    private:
        QVBoxLayout _layout;
        QTreeWidget _treeWidget;
        FilterLineEdit _searchEdit;
};

#endif // PROPERTIESWIDGET_H
