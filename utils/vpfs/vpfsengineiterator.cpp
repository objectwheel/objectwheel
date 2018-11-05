#include <vpfsengineiterator.h>
#include <vpfs.h>

VpfsEngineIterator::VpfsEngineIterator(QDir::Filters filters, const QStringList &filterNames)
    : QAbstractFileEngineIterator(filters, filterNames), index(-1)
{
}

VpfsEngineIterator::~VpfsEngineIterator()
{
}

QString VpfsEngineIterator::next()
{
    if (!hasNext())
        return QString();
    ++index;
    return currentFilePath();
}

bool VpfsEngineIterator::hasNext() const
{
    if (index == -1) {
        // Lazy initialization of the iterator
        Vpfs resource(path());
        if (!resource.isValid())
            return false;

        // Initialize and move to the next entry.
        entries = resource.children();
        index = 0;
    }

    return index < entries.size();
}

QString VpfsEngineIterator::currentFileName() const
{
    if (index <= 0 || index > entries.size())
        return QString();
    return entries.at(index - 1);
}
