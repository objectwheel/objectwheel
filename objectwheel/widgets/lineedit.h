#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>

class LineEdit final : public QLineEdit
{
    Q_OBJECT
    Q_DISABLE_COPY(LineEdit)
    Q_DECLARE_PRIVATE(QLineEdit)

public:
    enum VisualStyle { Search, Plain };

public:
    explicit LineEdit(VisualStyle visualStyle, QWidget* parent = nullptr);
    explicit LineEdit(QWidget* parent = nullptr);

    VisualStyle visualStyle() const;
    void setVisualStyle(const VisualStyle& visualStyle);

    QString oldText() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private slots:
    void onTextChange(const QString& text);

private:
    void paintEvent(QPaintEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    VisualStyle m_visualStyle;
    QString m_oldText;
    QString m_newText;
};

#endif // LINEEDIT_H
