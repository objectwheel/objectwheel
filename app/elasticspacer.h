#ifndef ELASTICSPACER_H
#define ELASTICSPACER_H

#include <QSpacerItem>

class ElasticSpacer : public QSpacerItem
{
public:
    ElasticSpacer(QWidget* watched, int w, int h,
                  QSizePolicy::Policy hPolicy = QSizePolicy::Minimum,
                  QSizePolicy::Policy vPolicy = QSizePolicy::Minimum);

protected:
    QSize sizeHint() const override;

private:
    QWidget* m_watched;
};

#endif // ELASTICSPACER_H