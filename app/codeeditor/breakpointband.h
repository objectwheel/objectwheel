#ifndef BREAKPOINTBAND_H
#define BREAKPOINTBAND_H

#include <QWidget>

class QmlCodeEditor;

class BreakpointBand : public QWidget
{
    Q_OBJECT

public:
    explicit BreakpointBand(QmlCodeEditor* editor, QWidget* parent = nullptr);
    int calculatedWidth() const;

private:
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

private:
    QmlCodeEditor* m_qmlCodeEditor;
};

#endif // BREAKPOINTBAND_H