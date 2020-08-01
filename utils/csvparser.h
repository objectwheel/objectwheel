#ifndef CSVPARSER_H
#define CSVPARSER_H

#include <QVector>

class CsvParser final
{
public:
    CsvParser(const QString& filePath);
    bool parse();

public:
    int rowCount() const;
    int columnCount() const;
    QVector<QStringList> rows() const;
    QString at(int row, int column) const;

private:
    QString cleanWord(QString word) const;

private:
    const QString m_filePath;
    int m_rowCount;
    int m_columnCount;
    QVector<QStringList> m_rows;
};

#endif // CSVPARSER_H
