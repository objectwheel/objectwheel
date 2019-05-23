#ifndef FOCUSLESSLINEEDIT_H
#define FOCUSLESSLINEEDIT_H

#include <QLineEdit>

class FocuslessLineEdit final : public QLineEdit
{
    Q_OBJECT
    Q_DISABLE_COPY(FocuslessLineEdit)
    Q_DECLARE_PRIVATE(QLineEdit)

public:
    explicit FocuslessLineEdit(QWidget* parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void paintEvent(QPaintEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
};

#endif // FOCUSLESSLINEEDIT_H
