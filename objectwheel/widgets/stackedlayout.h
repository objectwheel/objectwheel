#ifndef STACKEDLAYOUT_H
#define STACKEDLAYOUT_H

#include <QLayout>
#include <QPointer>
#include <QVariantAnimation>

class QVariantAnimation;
class StackedLayout final : public QLayout
{
    Q_OBJECT
    Q_DISABLE_COPY(StackedLayout)

public:
    enum SlidingDirection { Natural, Left, Right };

public:
    StackedLayout();
    explicit StackedLayout(QWidget* parent);
    explicit StackedLayout(QLayout* parentLayout);
    ~StackedLayout() override;

    int addWidget(QWidget* w);
    int insertWidget(int index, QWidget* w);

    using QLayout::widget;
    QWidget* widget(int) const;
    QWidget* currentWidget() const;
    int currentIndex() const;
    int count() const override;

    bool animationEnabled() const;
    void setAnimationEnabled(bool animationEnabled);
    int animationDuration() const;
    void setAnimationDuration(int animationDuration);
    QEasingCurve animationEasingCurve() const;
    void setAnimationEasingCurve(const QEasingCurve& animationEasingCurve);

    void addItem(QLayoutItem* item) override;
    QSize sizeHint() const override;
    QSize minimumSize() const override;
    QLayoutItem* itemAt(int) const override;
    QLayoutItem* takeAt(int) override;
    void setGeometry(const QRect& rect) override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;

public slots:
    void setCurrentWidget(QWidget* widget, SlidingDirection direction = Natural);
    void setCurrentIndex(int index, SlidingDirection direction = Natural);

private slots:
    void onAnimationValueChanged(const QVariant& value);
    void onAnimationStateChanged(QAbstractAnimation::State newState, QAbstractAnimation::State oldState);

signals:
    void widgetRemoved(int index);
    void currentChanged(int index);

private:
    int m_index;
    bool m_animationEnabled;
    QList<QLayoutItem*> m_list;
    QVariantAnimation* m_animation;
    QPointer<QWidget> m_prev, m_next;
};

#endif // STACKEDLAYOUT_H
