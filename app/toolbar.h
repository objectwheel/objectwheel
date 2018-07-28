#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QWidget>

class QBoxLayout;

class ToolBar : public QWidget
{
    Q_OBJECT
public:
    explicit ToolBar(QWidget* parent = nullptr);

public:
    void addStretch();
    void addSeparator();
    void addSpacing(int spacing);
    void addWidget(QWidget* widget);
    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

signals:
    void orientationChanged(Qt::Orientation orientation);

private:
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent* e) override;

private:
    QBoxLayout* m_layout;

};

#endif // TOOLBAR_H