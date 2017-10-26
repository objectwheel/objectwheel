#ifndef FILTERLINEEDIT_H
#define FILTERLINEEDIT_H

#include <QLineEdit>

class FilterLineEdit : public QLineEdit {
    public:
        explicit FilterLineEdit(QWidget *parent = 0);

    protected:
        void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
        void focusInEvent(QFocusEvent *e) Q_DECL_OVERRIDE;

    private:
        const Qt::FocusPolicy m_defaultFocusPolicy;
};

#endif // FILTERLINEEDIT_H
