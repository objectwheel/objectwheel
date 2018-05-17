#ifndef MARKBAND_H
#define MARKBAND_H

#include <QWidget>
#include <blockdata.h>

class QmlCodeEditor;

class MarkBand : public QWidget
{
    Q_OBJECT

public:
    explicit MarkBand(QmlCodeEditor* editor, QWidget* parent = nullptr);
    int calculatedWidth() const;

signals:
    void markActivated(const Mark& mark);

private:
    QSize sizeHint() const override;
    void leaveEvent(QEvent *e) override;
    void paintEvent(QPaintEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

private:
    QmlCodeEditor* m_qmlCodeEditor;
};

#endif // MARKBAND_H