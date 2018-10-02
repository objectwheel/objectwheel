#include <elasticspacer.h>
#include <QWidget>

ElasticSpacer::ElasticSpacer(QWidget* watched, int w, int h, QSizePolicy::Policy hPolicy, QSizePolicy::Policy vPolicy)
    : QSpacerItem(w, h, hPolicy, vPolicy)
    , m_watched(watched)
{
}

QSize ElasticSpacer::sizeHint() const
{
    return QSpacerItem::sizeHint() - m_watched->sizeHint();
}
