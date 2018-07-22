#ifndef TRANSPARENTCOMBOBOX_H
#define TRANSPARENTCOMBOBOX_H

#include <QComboBox>

class TransparentComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit TransparentComboBox(QWidget* parent = nullptr);

private:
    void paintEvent(QPaintEvent* e) override;
};

#endif // TRANSPARENTCOMBOBOX_H