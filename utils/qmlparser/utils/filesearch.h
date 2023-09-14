// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "utils_global.h"

#include "filepath.h"
#include "searchresultitem.h"

#include <QMap>
#include <QSet>
#include <QStack>
#include <QTextDocument>

#include <functional>

template <typename T>
class QFuture;

namespace Utils {

UTILS_EXPORT
std::function<FilePaths(const FilePaths &)> filterFilesFunction(const QStringList &filters,
                                                                const QStringList &exclusionFilters);

UTILS_EXPORT
QStringList splitFilterUiText(const QString &text);

UTILS_EXPORT
QString msgFilePatternLabel();

UTILS_EXPORT
QString msgExclusionPatternLabel();

enum class InclusionType {
    Included,
    Excluded
};

UTILS_EXPORT
QString msgFilePatternToolTip(InclusionType inclusionType = InclusionType::Included);

class UTILS_EXPORT FileIterator
{
public:
    class Item
    {
    public:
        Item() = default;
        Item(const FilePath &path)
            : filePath(path)
        {}
        FilePath filePath;
    };

    using value_type = Item;

    class const_iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = Item;
        using difference_type = std::ptrdiff_t;
        using pointer = const value_type*;
        using reference = const value_type&;

        const_iterator() = default;
        const_iterator(const FileIterator *parent, int id)
            : m_parent(parent), m_index(id)
        {}
        const_iterator(const const_iterator &) = default;
        const_iterator &operator=(const const_iterator &) = default;

        reference operator*() const { return m_parent->itemAt(m_index); }
        pointer operator->() const { return &m_parent->itemAt(m_index); }
        void operator++() { m_parent->advance(this); }
        bool operator==(const const_iterator &other) const
        {
            return m_parent == other.m_parent && m_index == other.m_index;
        }
        bool operator!=(const const_iterator &other) const { return !operator==(other); }

        const FileIterator *m_parent = nullptr;
        int m_index = -1; // -1 == end
    };

    virtual ~FileIterator() = default;
    const_iterator begin() const;
    const_iterator end() const;

    virtual int maxProgress() const = 0;
    virtual int currentProgress() const = 0;

    void advance(const_iterator *it) const;
    virtual const Item &itemAt(int index) const = 0;

protected:
    virtual void update(int requestedIndex) = 0;
    virtual int currentFileCount() const = 0;
};

class UTILS_EXPORT FileListIterator : public FileIterator
{
public:
    explicit FileListIterator(const FilePaths &fileList = {});

    int maxProgress() const override;
    int currentProgress() const override;

protected:
    void update(int requestedIndex) override;
    int currentFileCount() const override;
    const Item &itemAt(int index) const override;

private:
    const QList<Item> m_items;
    int m_maxIndex = -1;
};

class UTILS_EXPORT SubDirFileIterator : public FileIterator
{
public:
    SubDirFileIterator(const FilePaths &directories,
                       const QStringList &filters,
                       const QStringList &exclusionFilters);
    ~SubDirFileIterator() override;

    int maxProgress() const override;
    int currentProgress() const override;

protected:
    void update(int requestedIndex) override;
    int currentFileCount() const override;
    const Item &itemAt(int index) const override;

private:
    std::function<FilePaths(const FilePaths &)> m_filterFiles;
    QStack<FilePath> m_dirs;
    QSet<FilePath> m_knownDirs;
    QStack<qreal> m_progressValues;
    QStack<bool> m_processedValues;
    qreal m_progress;
    // Use heap allocated objects directly because we want references to stay valid even after resize
    QList<Item *> m_items;
};

UTILS_EXPORT QFuture<SearchResultItems> findInFiles(const QString &searchTerm,
    FileIterator *files,
    QTextDocument::FindFlags flags,
    const QMap<FilePath, QString> &fileToContentsMap = {});

UTILS_EXPORT QFuture<SearchResultItems> findInFilesRegExp(
    const QString &searchTerm,
    FileIterator *files,
    QTextDocument::FindFlags flags,
    const QMap<FilePath, QString> &fileToContentsMap = {});

UTILS_EXPORT QString expandRegExpReplacement(const QString &replaceText,
                                                       const QStringList &capturedTexts);
UTILS_EXPORT QString matchCaseReplacement(const QString &originalText,
                                                    const QString &replaceText);

} // namespace Utils
