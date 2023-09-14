// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "utils_global.h"

#include <QMetaType>
#include <QString>

class QTextCursor;
class QTextDocument;

namespace Utils {
namespace Text {

class UTILS_EXPORT Position
{
public:
    int line = 0; // 1-based
    int column = -1; // 0-based

    bool operator<(const Position &other) const
    { return line < other.line || (line == other.line && column < other.column); }
    bool operator==(const Position &other) const;

    bool operator!=(const Position &other) const { return !(operator==(other)); }

    bool isValid() const { return line > 0 && column >= 0; }

    static Position fromFileName(QStringView fileName, int &postfixPos);
    static Position fromPositionInDocument(const QTextDocument *document, int pos);
    static Position fromCursor(const QTextCursor &cursor);
};

class UTILS_EXPORT Range
{
public:
    int length(const QString &text) const;

    Position begin;
    Position end;

    bool operator<(const Range &other) const { return begin < other.begin; }
    bool operator==(const Range &other) const;

    bool operator!=(const Range &other) const { return !(operator==(other)); }
};

struct Replacement
{
    Replacement() = default;
    Replacement(int offset, int length, const QString &text)
        : offset(offset)
        , length(length)
        , text(text)
    {}

    int offset = -1;
    int length = -1;
    QString text;

    bool isValid() const { return offset >= 0 && length >= 0;  }
};
using Replacements = std::vector<Replacement>;

UTILS_EXPORT void applyReplacements(QTextDocument *doc, const Replacements &replacements);

// line is 1-based, column is 0-based
UTILS_EXPORT bool convertPosition(const QTextDocument *document,
                                            int pos,
                                            int *line, int *column);

// line and column are 1-based
UTILS_EXPORT int positionInText(const QTextDocument *textDocument, int line, int column);

UTILS_EXPORT QString textAt(QTextCursor tc, int pos, int length);

UTILS_EXPORT QTextCursor selectAt(QTextCursor textCursor, int line, int column, uint length);

UTILS_EXPORT QTextCursor flippedCursor(const QTextCursor &cursor);

UTILS_EXPORT QTextCursor wordStartCursor(const QTextCursor &cursor);
UTILS_EXPORT QString wordUnderCursor(const QTextCursor &cursor);

UTILS_EXPORT bool utf8AdvanceCodePoint(const char *&current);
UTILS_EXPORT int utf8NthLineOffset(const QTextDocument *textDocument,
                                             const QByteArray &buffer,
                                             int line);

UTILS_EXPORT QString utf16LineTextInUtf8Buffer(const QByteArray &utf8Buffer,
                                                         int currentUtf8Offset);

UTILS_EXPORT QDebug &operator<<(QDebug &stream, const Position &pos);

} // Text
} // Utils

Q_DECLARE_METATYPE(Utils::Text::Position)
Q_DECLARE_METATYPE(Utils::Text::Range)
