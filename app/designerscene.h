#ifndef FORMSCENE_H
#define FORMSCENE_H

#include <form.h>
#include <QPointer>
#include <QGraphicsScene>

class DesignerScene : public QGraphicsScene
{
    Q_OBJECT

    friend class ControlRemovingManager; // For removeControl()
    friend class ControlExposingManager; // For addForm()

public:
    explicit DesignerScene(QObject *parent = Q_NULLPTR);
    const QList<Form*>& forms() const;
    void setCurrentForm(Form* currentForm);
    Form* currentForm();
    bool snapping() const;
    void setSnapping(bool snapping);
    bool showOutlines() const;
    void setShowOutlines(bool value);

    QList<Control*> controlsAt(const QPointF& pos) const;
    QList<Control*> selectedControls() const;
    QPointF lastMousePos() const;
    bool stick() const;
    QVector<QLineF> guideLines() const;

public slots:
    void sweep();

private:
    void addForm(Form* form);
    void removeForm(Form* form);
    void removeControl(Control* control);

private:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
    void drawForeground(QPainter* painter, const QRectF& rect);

signals:
    void currentFormChanged(Form* currentForm);

private:
    bool m_snapping;
    bool m_showOutlines;
    QPointF m_lastMousePos;
    QList<Form*> m_forms;
    QPointer<Form> m_currentForm;
};

#endif // FORMSCENE_H
