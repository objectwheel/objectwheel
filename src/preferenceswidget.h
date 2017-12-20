#ifndef PREFERENCESWIDGET_H
#define PREFERENCESWIDGET_H

#include <QWidget>

class QLabel;
class QGridLayout;
class QListWidget;
class QScrollArea;
class FilterLineEdit;
class QDialogButtonBox;

class PreferencesWidget : public QWidget
{
        Q_OBJECT
    public:
        explicit PreferencesWidget(QWidget *parent = nullptr);

    protected:
        QSize sizeHint() const override;

    private slots:
        void filterList(const QString& text);

    private:
        QGridLayout* layout;
        QListWidget* listWidget;
        QScrollArea* scrollArea;
        FilterLineEdit* txtFilter;
        QLabel* lblTitle;
        QDialogButtonBox* bboxButtons;
};

#endif // PREFERENCESWIDGET_H