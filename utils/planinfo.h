#ifndef PLANINFO_H
#define PLANINFO_H

#include <utils_global.h>
#include <QColor>

class PlanInfo;

namespace PlanParser {
UTILS_EXPORT PlanInfo parse(const QByteArray& planData);
} // PlanParser

class UTILS_EXPORT PlanInfo final
{
    friend PlanInfo PlanParser::parse(const QByteArray&);

public:
    PlanInfo();

public:
    int rowCount() const;
    int columnCount() const;
    int columnForIdentifier(qint64 identifier) const;
    qint64 defaultPlan() const;
    QString at(int row, int column) const;
    QString badge(int column) const;
    qint64 identifier(int column) const;
    qreal price(int column) const;
    qreal annualPrice(int column) const;
    qreal tax(int column) const;
    qreal annualTax(int column) const;
    QColor badgeColor(int column, bool enabled = true) const;
    QColor headerColor(int column, bool enabled = true) const;
    QColor columnColor(int column, bool enabled = true) const;

private:
    int m_rowCount;
    int m_columnCount;
    qint64 m_defaultPlan;
    QVector<QVector<QString>> m_rows;
    QVector<QString> m_badges;
    QVector<qint64> m_identifiers;
    QVector<qreal> m_prices;
    QVector<qreal> m_annualPrices;
    QVector<qreal> m_taxes;
    QVector<qreal> m_annualTaxes;
    QVector<QColor> m_badgeColors;
    QVector<QColor> m_headerColors;
    QVector<QColor> m_columnColors;
};

#endif // PLANINFO_H
