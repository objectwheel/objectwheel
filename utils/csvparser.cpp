#include <csvparser.h>
#include <QFile>
#include <QTextStream>

CsvParser::CsvParser(const QString& filePath)
    : m_filePath(filePath)
    , m_rowCount(-1)
    , m_columnCount(-1)
{
}

bool CsvParser::parse()
{
    QFile file(m_filePath);
    if (!file.open(QFile::ReadOnly))
        return false;

    QTextStream stream(&file);
    stream.setCodec("UTF-8");

    QChar c;
    QString word;
    QStringList row;
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

    m_rowCount = m_rows.size();
    if (!m_rows.isEmpty())
        m_columnCount = m_rows.first().size();

    return true;
}

int CsvParser::rowCount() const
{
    return m_rowCount;
}

int CsvParser::columnCount() const
{
    return m_columnCount;
}

QVector<QStringList> CsvParser::rows() const
{
    return m_rows;
}

QString CsvParser::at(int row, int column) const
{
    return m_rows.at(row).at(column);
}

QString CsvParser::cleanWord(QString word) const
{
    if (word.startsWith(QLatin1Char('\"')))
        word.remove(0, 1);
    if (word.endsWith(QLatin1Char('\"')))
        word.chop(1);
    return word.replace(QLatin1String("\"\""), QLatin1String("\""));
}