#ifndef FORMSCENE_H
#define FORMSCENE_H

#include <form.h>
#include <QPointer>
#include <QGraphicsScene>

class DesignerScene : public QGraphicsScene
{
    Q_OBJECT

    friend class ControlRemovingManager; // For removeControl(), removeForm()
    friend class ControlCreationManager; // For addForm(), addControl()
    friend class ProjectExposingManager; // For addForm(), addControl()

public:
    explicit DesignerScene(QObject *parent = Q_NULLPTR);
    const QList<Form*>& forms() const;
    void setCurrentForm(Form* currentForm);
    Form* currentForm();

    QList<Control*> controlsAt(const QPointF& pos) const;
    QList<Control*> selectedControls() const;
    QPointF lastMousePos() const;
    void stick() const;
    QVector<QLineF> guideLines() const;

    static QColor highlightColor();
    static QPen pen(const QColor& color = highlightColor(), qreal width = 1, bool cosmetic = true);

public slots:
    void discharge();
    void centralize();

private:
    void addForm(Form* form);
    void addControl(Control* control, Control* parentControl);
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
    QPointF m_lastMousePos;
    QList<Control*> m_draggedControls;
    QList<Form*> m_forms;
    QPointer<Form> m_currentForm;
};

#endif // FORMSCENE_H
