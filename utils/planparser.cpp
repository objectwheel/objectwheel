#include <planparser.h>
#include <paintutils.h>
#include <QTextStream>

PlanParser::PlanParser()
    : m_rowCount(0)
    , m_columnCount(0)
{
}

void PlanParser::parse(const QByteArray& planData)
{
    *this = PlanParser();

    QTextStream stream(planData);
    stream.setCodec("UTF-8");
    QChar c;
    QString word;
    QVector<QString> row;
    bool inQuote = false;
    while (!stream.atEnd()) {
        stream >> c;
        if (!inQuote) {
            if (c == '\n' || c == ';') {
                row.append(cleanWord(word));
                word.clear();
                if (c == '\n') {
                    m_rows.append(row);
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
        row.append(cleanWord(word));
    if (!row.isEmpty())
        m_rows.append(row);

    for (int i = m_rows.size() - 1; i >= 0; i--) {
        const QVector<QString>& row = m_rows.at(i);
        if (row.size() < 1)
            continue;
        const QString& signature = row.first();
        if (!signature.startsWith(QLatin1Char('#')))
            continue;
        for (int j = 0; j < row.size(); j++) {
            if (signature == QLatin1String("#default") && !row.at(j).isEmpty()) {
                m_defaultPlan = m_rows.first().at(j);
            } else if (signature == QLatin1String("#header-color")) {
                m_headerColors.append(QColor(row.at(j)));
            } else if (signature == QLatin1String("#column-color")) {
                m_columnColors.append(QColor(row.at(j)));
            } else if (signature == QLatin1String("#badge-color")) {
                m_badgeColors.append(QColor(row.at(j)));
            } else if (signature == QLatin1String("#badge")) {
                m_badges.append(row.at(j));
            } else if (signature == QLatin1String("#price")) {
                bool ok = false;
                qreal val = row.at(j).toDouble(&ok);
                if (ok)
                    m_prices.append(val);
                else
                    m_prices.append(-1);
            } else if (signature == QLatin1String("#annual-price")) {
                bool ok = false;
                qreal val = row.at(j).toDouble(&ok);
                if (ok)
                    m_annualPrices.append(val);
                else
                    m_annualPrices.append(-1);
            }
        }
        m_rows.remove(i);
    }

    m_columnCount = 0;
    m_rowCount = m_rows.size();
    if (!m_rows.isEmpty())
        m_columnCount = m_rows.first().size();

    for (int i = 0; i < m_rows.size(); i++) {
        if (m_columnCount > m_rows[i].size())
            m_rows[i].append(QVector<QString>(m_columnCount - m_rows[i].size()));
    }
    if (m_columnCount > m_badges.size())
        m_badges.append(QVector<QString>(m_columnCount - m_badges.size()));
    if (m_columnCount > m_prices.size())
        m_prices.append(QVector<qreal>(m_columnCount - m_prices.size(), -1));
    if (m_columnCount > m_annualPrices.size())
        m_annualPrices.append(QVector<qreal>(m_columnCount - m_annualPrices.size(), -1));
    if (m_columnCount > m_badgeColors.size())
        m_badgeColors.append(QVector<QColor>(m_columnCount - m_badgeColors.size()));
    if (m_columnCount > m_headerColors.size())
        m_headerColors.append(QVector<QColor>(m_columnCount - m_headerColors.size()));
    if (m_columnCount > m_columnColors.size())
        m_columnColors.append(QVector<QColor>(m_columnCount - m_columnColors.size()));
}

int PlanParser::rowCount() const
{
    return m_rowCount;
}

int PlanParser::columnCount() const
{
    return m_columnCount;
}

QString PlanParser::defaultPlan() const
{
    return m_defaultPlan;
}

QString PlanParser::at(int row, int column) const
{
    return m_rows.at(row).at(column);
}

QString PlanParser::badge(int column) const
{
    return m_badges.at(column);
}

qreal PlanParser::price(int column) const
{
    return m_prices.at(column);
}

qreal PlanParser::annualPrice(int column) const
{
    return m_annualPrices.at(column);
}

QColor PlanParser::badgeColor(int column, bool enabled) const
{
    return enabled ? m_badgeColors.at(column)
                   : PaintUtils::disabledColor(m_badgeColors.at(column));
}

QColor PlanParser::headerColor(int column, bool enabled) const
{
    return enabled ? m_headerColors.at(column)
                   : PaintUtils::disabledColor(m_headerColors.at(column));
}

QColor PlanParser::columnColor(int column, bool enabled) const
{
    return enabled ? m_columnColors.at(column)
                   : PaintUtils::disabledColor(m_columnColors.at(column));
}

QString PlanParser::cleanWord(QString word) const
{
    if (word.startsWith(QLatin1Char('\"')))
        word.remove(0, 1);
    if (word.endsWith(QLatin1Char('\"')))
        word.chop(1);
    return word.replace(QLatin1String("\"\""), QLatin1String("\""));
}
