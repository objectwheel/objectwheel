#ifndef LINENUMBERBAND_H
#define LINENUMBERBAND_H

#include <QWidget>

class QmlCodeEditor;

struct ExtraAreaPaintEventData;

class LineNumberBand : public QWidget
{
    Q_OBJECT

public:
    explicit LineNumberBand(QmlCodeEditor* editor, QWidget* parent = nullptr);
    int calculatedWidth() const;

private:
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent* e) override;

private:
    QmlCodeEditor* m_qmlCodeEditor;
};

#endif // LINENUMBERBAND_H