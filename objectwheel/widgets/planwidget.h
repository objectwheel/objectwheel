#ifndef PLANWIDGET_H
#define PLANWIDGET_H

#include <QWidget>
#include <planinfo.h>

class PlanWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(PlanWidget)

public:
    explicit PlanWidget(QWidget* parent = nullptr);

    int radius() const;
    void setRadius(int radius);

    int spacing() const;
    void setSpacing(int spacing);

    int padding() const;
    void setPadding(int padding);

    PlanInfo planInfo() const;
    void setPlanInfo(const PlanInfo& planInfo);

    qint64 selectedPlan() const;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    int rowHeight() const;
    int headerHeight() const;
    int blockWidth() const;
    int columnWidth() const;
    QPen cosmeticPen(const QColor& color) const;

private:
    void changeEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    PlanInfo m_planInfo;
    int m_radius;
    int m_spacing;
    int m_padding;
};

#endif // PLANWIDGET_H
