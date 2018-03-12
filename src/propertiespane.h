#ifndef PROPERTIESPANE_H
#define PROPERTIESPANE_H

#include <QWidget>

class QVBoxLayout;
class QTreeWidget;
class FocuslessLineEdit;

class PropertiesPane : public QWidget
{
		Q_OBJECT

	public:
        explicit PropertiesPane(QWidget* parent = nullptr);

	public slots:
        void clear();
        void clearList();
        void refreshList();

    protected:
        QSize sizeHint() const override;
        bool eventFilter(QObject *watched, QEvent *event) override;

    private slots:
        void handleSelectionChange();
        void filterList(const QString& filter);

    private:
        QVBoxLayout* _layout;
        QTreeWidget* _treeWidget;
        FocuslessLineEdit* _searchEdit;
};

#endif // PROPERTIESPANE_H
