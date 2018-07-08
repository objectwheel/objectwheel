#ifndef FORM_H
#define FORM_H

#include <control.h>

class Form : public Control
{
    Q_OBJECT

    friend class ControlCreationManager; // For constructor
    friend class ProjectExposingManager; // For constructor

public:
    bool main() const;
    QRectF frameGeometry() const;

public:
    void setMain(bool value);

public slots:
    void centralize();

private:
    void resizeEvent(QGraphicsSceneResizeEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    explicit Form(const QString& url, Form* parent = nullptr);

private:
    bool m_main;
};


#endif // FORM_H