#ifndef FORMSCENE_H
#define FORMSCENE_H

#include <control.h>
#include <QGraphicsScene>
#include <QPointer>

class DesignerScene : public QGraphicsScene
{
        Q_OBJECT

    public:
        explicit DesignerScene(QObject *parent = Q_NULLPTR);
        const QList<Form*>& forms() const;
        void addForm(Form* form);
        void removeForm(Form* form);
        void removeControl(Control* control);
        void removeChildControlsOnly(Control* parent);
        void setMainForm(Form* mainForm);
        Form* mainForm();
        bool snapping() const;
        void setSnapping(bool snapping);
        bool showOutlines() const;
        void setShowOutlines(bool value);

        QList<Control*> selectedControls() const;
        QPointF lastMousePos() const;
        bool stick() const;
        QVector<QLineF> guideLines() const;

    public slots:
        void clearScene();

    private:
        void mousePressEvent(QGraphicsSceneMouseEvent* event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
        void drawForeground(QPainter* painter, const QRectF& rect);

    signals:
        void aboutToRemove(Control* control);
        void controlRemoved(Control* control);
        void mainFormChanged(Control* mainForm);

    private:
        bool m_snapping;
        bool m_showOutlines;
        QPointF m_lastMousePos;
        QList<Form*> m_forms;
        QPointer<Form> m_mainForm;
};

#endif // FORMSCENE_H
