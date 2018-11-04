#ifndef VPFS_ITERATOR_P_H
#define VPFS_ITERATOR_P_H

#include <private/qabstractfileengine_p.h>
#include <QDir>

class VpfsFileEngineIteratorPrivate;
class VpfsFileEngineIterator : public QAbstractFileEngineIterator
{
public:
    VpfsFileEngineIterator(QDir::Filters filters, const QStringList &filterNames);
    ~VpfsFileEngineIterator();

    QString next() override;
    bool hasNext() const override;

    QString currentFileName() const override;

private:
    mutable QStringList entries;
    mutable int index;
};

#endif
