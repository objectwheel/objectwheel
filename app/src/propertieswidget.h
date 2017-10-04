#ifndef PROPERTIESWIDGET_H
#define PROPERTIESWIDGET_H

#include <listwidget.h>

#include <QWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QLineEdit>

class PropertiesWidget : public QWidget
{
		Q_OBJECT

	public:
		explicit PropertiesWidget(QWidget *parent = 0);
		const QColor& color() const;
		void setColor(const QColor& color);

	public slots:
        void clearList();
        void refreshList();
        void handleSelectionChange();

    protected:
        virtual QSize sizeHint() const override;

    private:
        QColor _color;
        QVBoxLayout _layout;
        ListWidget _listWidget;
        QLineEdit _searchEdit;
};

#endif // PROPERTIESWIDGET_H
