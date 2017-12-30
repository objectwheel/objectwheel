#ifndef FORMSPANE_H
#define FORMSPANE_H

#include <QWidget>

class FlatButton;
class QVBoxLayout;
class QHBoxLayout;
class QListWidget;
class MainWindow;

class FormsPane : public QWidget
{
        Q_OBJECT

    public:
        explicit FormsPane(MainWindow* parent);
        void setCurrentForm(int index);

    protected:
        QSize sizeHint() const override;

    private slots:
        void addButtonClicked();
        void removeButtonClicked();
        void handleDatabaseChange();
        void handleCurrentFormChange();

    signals:
        void currentFormChanged();

    private:
        QVBoxLayout* _layout;
        QHBoxLayout* _buttonLayout;
        QListWidget* _listWidget;
        FlatButton* _addButton;
        FlatButton* _removeButton;
};

#endif // FORMSPANE_H
