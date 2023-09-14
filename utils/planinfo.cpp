#include <planinfo.h>
#include <paintutils.h>
#include <QTextStream>

namespace PlanParser {
namespace Internal {

QString cleanWord(QString word)
{
    if (word.startsWith(QLatin1Char('\"')))
        word.remove(0, 1);
    if (word.endsWith(QLatin1Char('\"')))
        word.chop(1);
    return word.replace(QLatin1String("\"\""), QLatin1String("\""));
}
} // Internal

PlanInfo parse(const QByteArray& planData)
{
    PlanInfo info;
    QTextStream stream(planData);
    stream.setEncoding(QStringConverter::Utf8);
    QChar c;
    QString word;
    QVector<QString> row;
    bool inQuote = false;
    while (!stream.atEnd()) {
        stream >> c;
        if (!inQuote) {
            if (c == '\n' || c == ';') {
                row.append(Internal::cleanWord(word));
                word.clear();
                if (c == '\n') {
                    info.m_rows.append(row);
                    row.clear();
                }
                continue;
            }
            if (c == '\r')
                continue;
        }
        if (c == '\"')
            inQuote = !inQuote;
        word.append(c);
    }
    if (!word.isEmpty())
        row.append(Internal::cleanWord(word));
    if (!row.isEmpty())
        info.m_rows.append(row);

    int defaultIndex = 0;
    for (int i = info.m_rows.size() - 1; i >= 0; i--) {
        const QVector<QString>& row = info.m_rows.at(i);
        if (row.size() < 1)
            continue;
        const QString& signature = row.first();
        if (!signature.startsWith(QLatin1Char('#')))
            continue;
        for (int j = 0; j < row.size(); j++) {
            if (signature == QLatin1String("#default")) {
                if (!row.at(j).isEmpty())
                    defaultIndex = j;
            } else if (signature == QLatin1String("#header-color")) {
                info.m_headerColors.append(QColor(row.at(j)));
            } else if (signature == QLatin1String("#column-color")) {
                info.m_columnColors.append(QColor(row.at(j)));
            } else if (signature == QLatin1String("#badge-color")) {
                info.m_badgeColors.append(QColor(row.at(j)));
            } else if (signature == QLatin1String("#badge")) {
                info.m_badges.append(row.at(j));
            } else if (signature == QLatin1String("#trial-period")) {
                bool ok = false;
                qint64 val = row.at(j).toLongLong(&ok);
                if (ok)
                    info.m_trialPeriods.append(val);
                else
                    info.m_trialPeriods.append(-1);
            } else if (signature == QLatin1String("#identifier")) {
                bool ok = false;
                qint64 val = row.at(j).toLongLong(&ok);
                if (ok)
                    info.m_identifiers.append(val);
                else
                    info.m_identifiers.append(-1);
            } else if (signature == QLatin1String("#price")) {
                bool ok = false;
                qreal val = row.at(j).toDouble(&ok);
                if (ok)
                    info.m_prices.append(val);
                else
                    info.m_prices.append(-1);
            } else if (signature == QLatin1String("#annual-price")) {
                bool ok = false;
                qreal val = row.at(j).toDouble(&ok);
                if (ok)
                    info.m_annualPrices.append(val);
                else
                    info.m_annualPrices.append(-1);
            } else if (signature == QLatin1String("#tax")) {
                bool ok = false;
                qreal val = row.at(j).toDouble(&ok);
                if (ok)
                    info.m_taxes.append(val);
                else
                    info.m_taxes.append(-1);
            } else if (signature == QLatin1String("#annual-tax")) {
                bool ok = false;
                qreal val = row.at(j).toDouble(&ok);
                if (ok)
                    info.m_annualTaxes.append(val);
                else
                    info.m_annualTaxes.append(-1);
            }
        }
        info.m_rows.remove(i);
    }

    if (defaultIndex > 0 && info.m_identifiers.size() > defaultIndex)
        info.m_defaultPlan = info.m_identifiers.at(defaultIndex);

    info.m_columnCount = 0;
    info.m_rowCount = info.m_rows.size();
    if (!info.m_rows.isEmpty())
        info.m_columnCount = info.m_rows.first().size();

    for (int i = 0; i < info.m_rows.size(); i++) {
        if (info.m_columnCount > info.m_rows[i].size())
            info.m_rows[i].append(QVector<QString>(info.m_columnCount - info.m_rows[i].size()));
    }
    if (info.m_columnCount > info.m_badges.size())
        info.m_badges.append(QVector<QString>(info.m_columnCount - info.m_badges.size()));
    if (info.m_columnCount > info.m_trialPeriods.size())
        info.m_trialPeriods.append(QVector<qint64>(info.m_columnCount - info.m_trialPeriods.size(), -1));
    if (info.m_columnCount > info.m_identifiers.size())
        info.m_identifiers.append(QVector<qint64>(info.m_columnCount - info.m_identifiers.size(), -1));
    if (info.m_columnCount > info.m_prices.size())
        info.m_prices.append(QVector<qreal>(info.m_columnCount - info.m_prices.size(), -1));
    if (info.m_columnCount > info.m_annualPrices.size())
        info.m_annualPrices.append(QVector<qreal>(info.m_columnCount - info.m_annualPrices.size(), -1));
    if (info.m_columnCount > info.m_taxes.size())
        info.m_taxes.append(QVector<qreal>(info.m_columnCount - info.m_taxes.size(), -1));
    if (info.m_columnCount > info.m_annualTaxes.size())
        info.m_annualTaxes.append(QVector<qreal>(info.m_columnCount - info.m_annualTaxes.size(), -1));
    if (info.m_columnCount > info.m_badgeColors.size())
        info.m_badgeColors.append(QVector<QColor>(info.m_columnCount - info.m_badgeColors.size()));
    if (info.m_columnCount > info.m_headerColors.size())
        info.m_headerColors.append(QVector<QColor>(info.m_columnCount - info.m_headerColors.size()));
    if (info.m_columnCount > info.m_columnColors.size())
        info.m_columnColors.append(QVector<QColor>(info.m_columnCount - info.m_columnColors.size()));
    return info;
}
} // PlanParser

PlanInfo::PlanInfo()
    : m_rowCount(0)
    , m_columnCount(0)
    , m_defaultPlan(0)
{
}

int PlanInfo::rowCount() const
{
    return m_rowCount;
}

int PlanInfo::columnCount() const
{
    return m_columnCount;
}

int PlanInfo::columnForIdentifier(qint64 identifier) const
{
    for (int i = 0; i < m_identifiers.size(); ++i) {
        if (m_identifiers.at(i) == identifier)
            return i;
    }
    return -1;
}

qint64 PlanInfo::defaultPlan() const
{
    return m_defaultPlan;
}

QString PlanInfo::at(int row, int column) const
{
    return m_rows.at(row).at(column);
}

QString PlanInfo::badge(int column) const
{
    return m_badges.at(column);
}

qint64 PlanInfo::trialPeriod(int column) const
{
    return m_trialPeriods.at(column);
}

qint64 PlanInfo::identifier(int column) const
{
    return m_identifiers.at(column);
}

qreal PlanInfo::price(int column) const
{
    return m_prices.at(column);
}

qreal PlanInfo::annualPrice(int column) const
{
    return m_annualPrices.at(column);
}

qreal PlanInfo::tax(int column) const
{
    return m_taxes.at(column);
}

qreal PlanInfo::annualTax(int column) const
{
    return m_annualTaxes.at(column);
}

QColor PlanInfo::badgeColor(int column, bool enabled) const
{
    return enabled ? m_badgeColors.at(column)
                   : PaintUtils::disabledColor(m_badgeColors.at(column));
}

QColor PlanInfo::headerColor(int column, bool enabled) const
{
    return enabled ? m_headerColors.at(column)
                   : PaintUtils::disabledColor(m_headerColors.at(column));
}

QColor PlanInfo::columnColor(int column, bool enabled) const
{
    return enabled ? m_columnColors.at(column)
                   : PaintUtils::disabledColor(m_columnColors.at(column));
}
