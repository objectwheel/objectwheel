#ifndef FOCUSLESSLINEEDIT_H
#define FOCUSLESSLINEEDIT_H

#include <QLineEdit>

class FocuslessLineEditPrivate;

class FocuslessLineEdit final : public QLineEdit
{
    Q_OBJECT
    Q_DISABLE_COPY(FocuslessLineEdit)
    Q_DECLARE_PRIVATE(FocuslessLineEdit)

public:
    explicit FocuslessLineEdit(QWidget* parent = nullptr);

private:
    void paintEvent(QPaintEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    QSize sizeHint() const override;
};

#endif // FOCUSLESSLINEEDIT_H
