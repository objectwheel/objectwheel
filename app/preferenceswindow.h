#ifndef PREFERENCESWINDOW_H
#define PREFERENCESWINDOW_H

#include <QWidget>

class QLabel;
class QGridLayout;
class QListWidget;
class QScrollArea;
class FocuslessLineEdit;
class QDialogButtonBox;

class PreferencesWindow : public QWidget
{
    Q_OBJECT

    friend class WindowManager;

public:
    explicit PreferencesWindow(QWidget *parent = nullptr);

private slots:
    void filterList(const QString& text);

signals:
    void done();

private:
     QSize sizeHint() const override;

private:
    QGridLayout* m_layout;
    QListWidget* m_listWidget;
    QScrollArea* m_scrollArea;
    FocuslessLineEdit* m_txtFilter;
    QLabel* m_lblTitle;
    QDialogButtonBox* m_bboxButtons;
};

#endif // PREFERENCESWINDOW_H