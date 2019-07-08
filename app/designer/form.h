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
    enum { Type = UserType + 2 };

public:
    int type() const override;
    QRectF frameGeometry() const;

private:
    void paintFrame(QPainter* painter);
    void paintHeadline(QPainter* painter);
    void paintGridViewDots(QPainter* painter, int gridSize);
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    explicit Form(const QString& dir, Form* parent = nullptr);
};


#endif // FORM_H