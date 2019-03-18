#ifndef SMARTSPACER_H
#define SMARTSPACER_H

#include <QWidget>

class SmartSpacer : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(SmartSpacer)

public:
    SmartSpacer(Qt::Orientation orientation, const QList<QWidget*>& watchedWidgets, int baseSize,
                const QSize& minimumSizeHint, QSizePolicy::Policy hPolicy = QSizePolicy::Minimum,
                QSizePolicy::Policy vPolicy = QSizePolicy::Minimum, QWidget* parent = nullptr);

protected:
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    const int m_baseSize;
    const QSize m_minimumSizeHint;
    const Qt::Orientation m_orientation;
    const QList<QWidget*> m_watchedWidgets;
};

#endif // SMARTSPACER_H