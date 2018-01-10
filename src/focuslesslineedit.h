#ifndef FOCUSLESSLINEEDIT_H
#define FOCUSLESSLINEEDIT_H

#include <QLineEdit>

class FocuslessLineEdit : public QLineEdit
{
        Q_OBJECT

    public:
        explicit FocuslessLineEdit(QWidget* parent = nullptr);

    protected:
        void focusInEvent(QFocusEvent* event) override;
        void mousePressEvent(QMouseEvent* event) override;
};

#endif // FOCUSLESSLINEEDIT_H
