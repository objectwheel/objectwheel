#ifndef PLANWIDGET_H
#define PLANWIDGET_H

#include <QWidget>
#include <planparser.h>

class PlanWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(PlanWidget)

public:
    explicit PlanWidget(QWidget* parent = nullptr);

    PlanParser planParser() const;

    int radius() const;
    void setRadius(int radius);

    int spacing() const;
    void setSpacing(int spacing);

    int padding() const;
    void setPadding(int padding);

    QString selectedPlan() const;
    void setSelectedPlan(const QString& defaultPlan);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void setPlanData(const QByteArray& data);

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
    PlanParser m_planParser;
    int m_radius;
    int m_spacing;
    int m_padding;
};

#endif // PLANWIDGET_H
