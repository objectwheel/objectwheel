#include <smartspacer.h>

SmartSpacer::SmartSpacer(Qt::Orientation orientation, const QList<QWidget*>& watchedWidgets,
                         int baseSize, const QSize& minimumSizeHint, QSizePolicy::Policy hPolicy,
                         QSizePolicy::Policy vPolicy, QWidget* parent) : QWidget(parent)
  , m_baseSize(baseSize)
  , m_minimumSizeHint(minimumSizeHint)
  , m_orientation(orientation)
  , m_watchedWidgets(watchedWidgets)
{
    setSizePolicy(hPolicy, vPolicy);
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

QSize SmartSpacer::sizeHint() const
{
    QSize totalSizeHint(m_orientation == Qt::Vertical ? 0 : m_baseSize,
                        m_orientation == Qt::Horizontal ? 0 : m_baseSize);
    for (QWidget* watched : m_watchedWidgets) {
        totalSizeHint += QSize(m_orientation == Qt::Vertical ? 0 : watched->sizeHint().width(),
                               m_orientation == Qt::Horizontal ? 0 : watched->sizeHint().height());
    }
    return totalSizeHint;
}

QSize SmartSpacer::minimumSizeHint() const
{
    return m_minimumSizeHint;
}
