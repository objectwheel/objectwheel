#ifndef TRANSPARENTCOMBOBOX_H
#define TRANSPARENTCOMBOBOX_H

#include <QComboBox>

class TransparentComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit TransparentComboBox(QWidget* parent = nullptr);

private:
    void enterEvent(QEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    bool m_hoverOver;
};

#endif // TRANSPARENTCOMBOBOX_H