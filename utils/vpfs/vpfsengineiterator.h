#ifndef VPFSITERATOR_H
#define VPFSITERATOR_H

#include <private/qabstractfileengine_p.h>
#include <QDir>

class VpfsEngineIteratorPrivate;
class VpfsEngineIterator : public QAbstractFileEngineIterator
{
public:
    VpfsEngineIterator(QDir::Filters filters, const QStringList &filterNames);
    ~VpfsEngineIterator();

    QString next() override;
    bool hasNext() const override;

    QString currentFileName() const override;

private:
    mutable QStringList entries;
    mutable int index;
};

#endif // VPFSITERATOR_H
