#ifndef FORMSCENE_H
#define FORMSCENE_H

#include <form.h>
#include <QPointer>
#include <QGraphicsScene>

class DesignerScene final : public QGraphicsScene
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGraphicsScene)

    friend class ControlRemovingManager; // For removeControl(), removeForm()
    friend class ControlCreationManager; // For addForm(), addControl()
    friend class ProjectExposingManager; // For addForm(), addControl()

public:
    explicit DesignerScene(QObject* parent = nullptr);
    const QList<Form*>& forms() const;
    void setCurrentForm(Form* currentForm);
    Form* currentForm();
    qreal zoomLevel() const;

    QList<DesignerItem*> selectedItems() const;

    void unsetCursor();
    void setCursor(Qt::CursorShape cursor);

    QList<Control*> controlsAt(const QPointF& pos) const;
    QList<Control*> selectedControls() const;
    QList<DesignerItem*> draggedResizedSelectedItems() const;
    QVector<QLineF> guidelines() const;

    static int startDragDistance();
    static QColor outlineColor();
    static QPointF snapPosition(qreal x, qreal y);
    static QPointF snapPosition(const QPointF& pos);
    static QSizeF snapSize(qreal x, qreal y, qreal w, qreal h);
    static QSizeF snapSize(const QPointF& pos,const QSizeF& size);
    static QRectF boundingRect(const QList<DesignerItem*>& items);
    static QPen pen(const QColor& color = outlineColor(), qreal width = 1, bool cosmetic = true);

    void paintOutline(QPainter* painter, const QRectF& rect);

public slots:
    void discharge();

private:
    void addForm(Form* form);
    void removeForm(Form* form);
    void removeControl(Control* control);

private:
    void drawForeground(QPainter* painter, const QRectF& rect);

signals:
    void currentFormChanged(Form* currentForm);

private:
    QList<Form*> m_forms;
    QPointer<Form> m_currentForm;
};

#endif // FORMSCENE_H
