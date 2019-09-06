#ifndef DESIGNERVIEW_H
#define DESIGNERVIEW_H

#include <QGraphicsView>

class QmlCodeEditorWidget;
class DesignerScene;
class QToolBar;
class QToolButton;
class QComboBox;
class Control;
class SignalChooserDialog;
class QMenu;
class AnchorEditor;

class DesignerView final : public QGraphicsView
{
    Q_OBJECT
    Q_DISABLE_COPY(DesignerView)

    enum Panning {
        NotStarted,
        MouseWheelStarted,
        SpaceKeyStarted
    };

public:
    explicit DesignerView(QWidget* parent = nullptr);

    DesignerScene* scene() const;

    void stopPanning(QEvent* event);
    void startPanning(QEvent* event);

    void keyReleaseEvent(QKeyEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

public slots:
    void discharge();

protected:
    QSize sizeHint() const override;

private:
    void setZoomLevel(qreal zoomLevel);

private:
    AnchorEditor* m_anchorEditor;
    Panning m_panningState;
    QPoint m_panningStartPosition;
};

#endif // DESIGNERVIEW_H
