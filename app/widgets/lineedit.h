#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>

class LineEdit final : public QLineEdit
{
    Q_OBJECT
    Q_DISABLE_COPY(LineEdit)
    Q_DECLARE_PRIVATE(QLineEdit)

public:
    explicit LineEdit(QWidget* parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void paintEvent(QPaintEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
};

#endif // LINEEDIT_H
