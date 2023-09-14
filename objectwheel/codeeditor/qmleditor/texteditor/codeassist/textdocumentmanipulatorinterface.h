// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

//#include <texteditor/snippets/snippetparser.h>

#include <QTextCursor>

class QChar;
class QString;
class QTextCursor;

namespace TextEditor {

class TextDocumentManipulatorInterface
{
public:
    virtual ~TextDocumentManipulatorInterface() = default;

    virtual int currentPosition() const = 0;
    virtual int positionAt(TextPositionOperation textPositionOperation) const = 0;
    virtual QChar characterAt(int position) const = 0;
    virtual QString textAt(int position, int length) const = 0;
    virtual QTextCursor textCursorAt(int position) const = 0;

    virtual void setCursorPosition(int position) = 0;
    virtual void setAutoCompleteSkipPosition(int position) = 0;
    virtual bool replace(int position, int length, const QString &text) = 0;
//    virtual void insertCodeSnippet(int position,
//                                   const QString &text,
//                                   const SnippetParser &parse) = 0;
    virtual void paste() = 0;
    virtual void encourageApply() = 0;
    virtual void autoIndent(int position, int length) = 0;
};

} // namespace TextEditor
