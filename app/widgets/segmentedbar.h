#ifndef SEGMENTEDBAR_H
#define SEGMENTEDBAR_H

#include <QWidget>

class QStyleOptionButton;
class SegmentedBar final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(SegmentedBar)

public:
    explicit SegmentedBar(QWidget* parent = nullptr);

    void clear();

    QSize iconSize() const;
    void setIconSize(const QSize& iconSize);

    QAction* actionAt(int x, int y) const;
    QAction* actionAt(const QPoint& p) const;

    QAction* addAction(const QIcon& icon);
    QAction* addAction(const QString& text = QString());
    QAction* addAction(const QString& text, const QIcon& icon);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    qreal cellWidth() const;
    QPoint adjustedMenuPosition(QAction* action);
    void initStyleOption(QAction* action, QStyleOptionButton* option) const;

private:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void actionEvent(QActionEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    bool event(QEvent* event) override;

signals:
    void actionTriggered(QAction* action);
    void iconSizeChanged(const QSize& iconSize);

private:
    QSize m_iconSize;
};

#endif // SEGMENTEDBAR_H
