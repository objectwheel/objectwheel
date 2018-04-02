#ifndef FORMSPANE_H
#define FORMSPANE_H

#include <QWidget>

class QLabel;
class QFrame;
class FlatButton;
class QVBoxLayout;
class QHBoxLayout;
class QListWidget;
class MainWindow;
class DesignerScene;

class FormsPane : public QWidget
{
        Q_OBJECT

    public:
        explicit FormsPane(DesignerScene* designerScene, QWidget* parent = nullptr);
        void setCurrentForm(int index);

    public slots:
        void reset();

    protected:
        QSize sizeHint() const override;

    private slots:
        void addButtonClicked();
        void removeButtonClicked();
        void handleDatabaseChange();
        void handleCurrentFormChange();

    protected:
        bool eventFilter(QObject* watched, QEvent* event) override;

    signals:
        void currentFormChanged();

    private:
        DesignerScene* m_designerScene;
        QVBoxLayout* _layout,* _innerLayout;
        QFrame* _innerWidget;
        QHBoxLayout* _buttonLayout;
        QLabel* _header;
        QListWidget* _listWidget;
        FlatButton* _addButton;
        FlatButton* _removeButton;
};

#endif // FORMSPANE_H
