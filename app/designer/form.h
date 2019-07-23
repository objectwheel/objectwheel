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
    enum { Type = UserType + 3 };
    int type() const override;

private:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void paintFormFrame(QPainter* painter);
    void paintGridViewDots(QPainter* painter, int gridSize);
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    explicit Form(const QString& dir, Form* parent = nullptr);
};


#endif // FORM_H