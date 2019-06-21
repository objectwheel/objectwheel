#ifndef SEGMENTEDBUTTON_H
#define SEGMENTEDBUTTON_H

#include <QWidget>

class SegmentedButton final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(SegmentedButton)

public:
    explicit SegmentedButton(QWidget* parent = nullptr);

    void clear();

    QSize iconSize() const;
    void setIconSize(const QSize& iconSize);

    QAction* actionAt(int x, int y) const;
    QAction* actionAt(const QPoint& p) const;

    QAction* addAction(const QIcon& icon);
    QAction* addAction(const QString& text);
    QAction* addAction(const QString& text, const QIcon& icon);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    qreal cellWidth() const;

private:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void actionEvent(QActionEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

signals:
    void actionTriggered(QAction* action);
    void iconSizeChanged(const QSize& iconSize);

private:
    QSize m_iconSize;
};

#endif // SEGMENTEDBUTTON_H
