#ifndef DESIGNERPANE_H
#define DESIGNERPANE_H

#include <QWidget>

class DesignerPane : public QWidget
{
    Q_OBJECT
public:
    explicit DesignerPane(QWidget *parent = nullptr);

    void stopPanning(QEvent* event);
    void startPanning(QEvent* event);

    void keyReleaseEvent(QKeyEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

protected:
    QSize sizeHint() const override;
    void resizeEvent(QResizeEvent* event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void setZoomLevel(qreal zoomLevel);

private:
    enum Panning{ NotStarted, MouseWheelStarted, SpaceKeyStarted };
    Panning m_isPanning = Panning::NotStarted;
    QPoint m_panningStartPosition;
};

#endif // DESIGNERPANE_H
