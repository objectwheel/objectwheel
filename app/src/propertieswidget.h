#ifndef PROPERTIESWIDGET_H
#define PROPERTIESWIDGET_H

#include <listwidget.h>

#include <QWidget>
#include <QPair>
#include <QMetaProperty>

class QVBoxLayout;
class LineEdit;

class PropertiesWidget : public QWidget
{
		Q_OBJECT

    private:
        ListWidget* m_ListWidget;
		QVBoxLayout* m_Layout;
		LineEdit* m_SearchEdit;
        QColor m_Color;

	public:
		explicit PropertiesWidget(QWidget *parent = 0);
		const QColor& color() const;
		void setColor(const QColor& color);

	protected:
		void fixItemsGeometry();
        void showEvent(QShowEvent *event) override;
        virtual QSize sizeHint() const override;

	public slots:
        void clearList();
        void refreshList();
        void handleSelectionChange();
};

#endif // PROPERTIESWIDGET_H
