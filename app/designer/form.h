#ifndef FORM_H
#define FORM_H

#include <control.h>

class Form final : public Control
{
    Q_OBJECT
    Q_DISABLE_COPY(Form)

    friend class ControlCreationManager; // For constructor
    friend class ProjectExposingManager; // For constructor

public:
    enum { Type = UserType + 4 };
    int type() const override;

private:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void paintBackground(QPainter* painter);
    void paintForeground(QPainter* painter);
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    explicit Form(Control* parent = nullptr);
};

#endif // FORM_H
