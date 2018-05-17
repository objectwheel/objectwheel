#ifndef BRACKETBAND_H
#define BRACKETBAND_H

#include <QWidget>

class QmlCodeEditor;

class BracketBand : public QWidget
{
    Q_OBJECT

public:
    explicit BracketBand(QmlCodeEditor* editor, QWidget* parent = nullptr);
    int calculatedWidth() const;
    void updateCollapser();

private slots:
    void updateData();

private:
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent* e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void leaveEvent(QEvent *event) override;

private:
    QmlCodeEditor* m_qmlCodeEditor;
};

#endif // BRACKETBAND_H