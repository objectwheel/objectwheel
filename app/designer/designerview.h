#ifndef DESIGNERVIEW_H
#define DESIGNERVIEW_H

#include <QGraphicsView>

class DesignerScene;
class DesignerView final : public QGraphicsView
{
    Q_OBJECT
    Q_DISABLE_COPY(DesignerView)
    Q_DECLARE_PRIVATE(QGraphicsView)

    enum Panning {
        NotStarted,
        MouseWheelStarted,
        SpaceKeyStarted
    };

public:
    explicit DesignerView(QWidget* parent = nullptr);

    DesignerScene* scene() const;

    Qt::MouseButton mousePressButton() const;

private:
    void startPanning(QEvent* event);
    void stopPanning(QEvent* event);
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    Panning m_panningState;
    QPoint m_panningStartPosition;
};

#endif // DESIGNERVIEW_H
