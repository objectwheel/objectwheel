#ifndef FORM_H
#define FORM_H

#include <control.h>

class Form : public Control
{
        Q_OBJECT

    public:
        explicit Form(const QString& url, Form* parent = nullptr);
        bool main() const;
        QRectF frameGeometry() const;

    public:
        void setMain(bool value);

    private:
        void resizeEvent(QGraphicsSceneResizeEvent* event) override;
        void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    private:
        bool m_main;
};


#endif // FORM_H