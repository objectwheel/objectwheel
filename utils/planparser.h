#ifndef PLANPARSER_H
#define PLANPARSER_H

#include <QColor>

class PlanParser final
{
public:
    PlanParser();

    void parse(const QByteArray& planData);

public:
    int rowCount() const;
    int columnCount() const;
    QString defaultPlan() const;
    QString at(int row, int column) const;
    QString badge(int column) const;
    qreal price(int column) const;
    qreal annualPrice(int column) const;
    QColor badgeColor(int column, bool enabled = true) const;
    QColor headerColor(int column, bool enabled = true) const;
    QColor columnColor(int column, bool enabled = true) const;

private:
    QString cleanWord(QString word) const;

private:
    int m_rowCount;
    int m_columnCount;
    QString m_defaultPlan;
    QVector<QVector<QString>> m_rows;
    QVector<QString> m_badges;
    QVector<qreal> m_prices;
    QVector<qreal> m_annualPrices;
    QVector<QColor> m_badgeColors;
    QVector<QColor> m_headerColors;
    QVector<QColor> m_columnColors;
};

#endif // PLANPARSER_H
