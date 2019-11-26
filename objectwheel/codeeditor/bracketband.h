#ifndef BRACKETBAND_H
#define BRACKETBAND_H

#include <QWidget>
#include <QTextBlock>

class QmlCodeEditor;

class BracketBand : public QWidget
{
    Q_OBJECT

public:
    explicit BracketBand(QmlCodeEditor* editor, QWidget* parent = nullptr);
    int calculatedWidth() const;
    bool toggleFold(const QPoint& pos) const;

private slots:
    void updateData();

private:
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

private:
    QmlCodeEditor* m_qmlCodeEditor;
};

#endif // BRACKETBAND_H