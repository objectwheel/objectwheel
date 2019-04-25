#ifndef FORM_H
#define FORM_H

#include <control.h>

class Form : public Control
{
    Q_OBJECT

    friend class ControlCreationManager; // For constructor
    friend class ProjectExposingManager; // For constructor

public:
    QRectF frameGeometry() const;

private:
    void resizeEvent(QGraphicsSceneResizeEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    explicit Form(const QString& url, Form* parent = nullptr);
};


#endif // FORM_H