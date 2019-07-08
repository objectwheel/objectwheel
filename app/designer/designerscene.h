#ifndef FORMSCENE_H
#define FORMSCENE_H

#include <form.h>
#include <QPointer>
#include <QGraphicsScene>

class DesignerView;
class DesignerScene : public QGraphicsScene
{
    Q_OBJECT

    friend class ControlRemovingManager; // For removeControl(), removeForm()
    friend class ControlCreationManager; // For addForm(), addControl()
    friend class ProjectExposingManager; // For addForm(), addControl()

public:
    explicit DesignerScene(DesignerView* view, QObject* parent = nullptr);
    const QList<Form*>& forms() const;
    void setCurrentForm(Form* currentForm);
    Form* currentForm();
    DesignerView* view() const;

    QList<Control*> controlsAt(const QPointF& pos) const;
    QList<Control*> selectedControls() const;
    QPointF lastMousePos() const;
    void stick() const;
    QVector<QLineF> guideLines() const;

    static QColor outlineColor();
    static QPen pen(const QColor& color = outlineColor(), qreal width = 1, bool cosmetic = true);

public slots:
    void discharge();

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
    DesignerView* m_view;
    QPointF m_lastMousePos;
    QList<Control*> m_draggedControls;
    QList<Form*> m_forms;
    QPointer<Form> m_currentForm;
};

#endif // FORMSCENE_H
