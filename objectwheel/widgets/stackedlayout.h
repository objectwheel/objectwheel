#ifndef STACKEDLAYOUT_H
#define STACKEDLAYOUT_H

#include <QStackedLayout>

class StackedLayout final : public QStackedLayout
{
    Q_OBJECT
    Q_DISABLE_COPY(StackedLayout)

public:
    StackedLayout();
    explicit StackedLayout(QWidget* parent);
    explicit StackedLayout(QLayout* parentLayout);

    QSize sizeHint() const override;
    QSize minimumSize() const override;
};

#endif // STACKEDLAYOUT_H
