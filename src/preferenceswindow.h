#ifndef PREFERENCESWINDOW_H
#define PREFERENCESWINDOW_H

#include <QWidget>

class QLabel;
class QGridLayout;
class QListWidget;
class QScrollArea;
class FilterLineEdit;
class QDialogButtonBox;

class PreferencesWindow : public QWidget
{
        Q_OBJECT
    public:
        explicit PreferencesWindow(QWidget *parent = nullptr);

    private slots:
        void filterList(const QString& text);

    signals:
        void done();

    private:
        QGridLayout* _layout;
        QListWidget* _listWidget;
        QScrollArea* _scrollArea;
        FilterLineEdit* _txtFilter;
        QLabel* _lblTitle;
        QDialogButtonBox* _bboxButtons;
};

#endif // PREFERENCESWINDOW_H