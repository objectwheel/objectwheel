/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qmlhighlighter.h>

#define COLOR_KEYWORDS (QColor("#807F17"))
#define COLOR_QMLTYPE (QColor("#7F0F7E"))
#define COLOR_QTCLASS (QColor("#7F0F7E"))
#define COLOR_COMMMENTS (QColor("#0F7F12"))
#define COLOR_STRING (QColor("#0F7F12"))
#define COLOR_PROPERTIES (QColor("#7E0308"))
#define COLOR_FUNCTION (QColor("#020C7E"))
#define COLOR_NUMERICS (QColor("#369CF3"))

QmlHighlighter::QmlHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat.setForeground(COLOR_KEYWORDS);
    QStringList keywordPatterns;
    keywordPatterns << "\\bchar\\b" << "\\bclass\\b" << "\\bconst\\b"
                    << "\\bdouble\\b" << "\\benum\\b" << "\\bexplicit\\b"
                    << "\\bfriend\\b" << "\\binline\\b" << "\\bint\\b"
                    << "\\blong\\b" << "\\bnamespace\\b" << "\\boperator\\b"
                    << "\\bprivate\\b" << "\\bprotected\\b" << "\\bpublic\\b"
                    << "\\bshort\\b" << "\\bsignals\\b" << "\\bsigned\\b"
                    << "\\bslots\\b" << "\\bstatic\\b" << "\\bstruct\\b"
                    << "\\btemplate\\b" << "\\btypedef\\b" << "\\btypename\\b"
                    << "\\bunion\\b" << "\\bunsigned\\b" << "\\bvirtual\\b"
                    << "\\bvoid\\b" << "\\bvolatile\\b" << "\\bproperty\\b"
                    << "\\bvector4d\\b" << "\\bvector3d\\b" << "\\bvector2d\\b"
                    << "\\bvar\\b" << "\\bvariant\\b" << "\\burl\\b"
                    << "\\btime\\b" << "\\bstring\\b" << "\\bsize\\b"
                    << "\\brect\\b" << "\\breal\\b" << "\\bquaternion\\b"
                    << "\\bpoint\\b" << "\\bmatrix4x4\\b" << "\\blist\\b"
                    << "\\bfont\\b" << "\\benumeration\\b" << "\\bdate\\b"
                    << "\\bcolor\\b" << "\\baction\\b" << "\\bon\\b"
                    << "\\bimport\\b" << "\\breadonly\\b" << "\\bproperty\\b"
                    << "\\bsignal\\b" << "\\balias\\b" << "\\byield\\b"
                    << "\\btry\\b" << "\\bthrow\\b" << "\\bsuper\\b"
                    << "\\bnull\\b" << "\\blet\\b" << "\\bin\\b"
                    << "\\bgoto\\b" << "\\bfinally\\b" << "\\bexport\\b"
                    << "\\bbreak\\b" << "\\babstract\\b" << "\\bbyte\\b"
                    << "\\bdefault\\b" << "\\belse\\b" << "\\bextends\\b"
                    << "\\bif\\b" << "\\binstanceof\\b" << "\\bpackage\\b"
                    << "\\breturn\\b" << "\\bswitch\\b" << "\\bthrows\\b"
                    << "\\btypeof\\b" << "\\bwhile\\b" << "\\bawait\\b"
                    << "\\bcase\\b" << "\\bdelete\\b" << "\\bfalse\\b"
                    << "\\bfor\\b" << "\\bimplements\\b" << "\\bnative\\b"
                    << "\\bprivate\\b" << "\\bsynchronized\\b" << "\\btransient\\b"
                    << "\\bwith\\b" << "\\bboolean\\b" << "\\bcatch\\b"
                    << "\\bcontinue\\b" << "\\bdo\\b" << "\\beval\\b"
                    << "\\bfinal\\b" << "\\bfunction\\b" << "\\binterface\\b"
                    << "\\bnew\\b" << "\\bthis\\b" << "\\btrue\\b" ;

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    numericFormat.setForeground(COLOR_NUMERICS);
    rule.pattern = QRegExp("[0-9]+\\.?[0-9]*");
    rule.format = numericFormat;
    highlightingRules.append(rule);

    typeFormat.setForeground(COLOR_QMLTYPE);
    rule.pattern = QRegExp("\\b[A-Z][A-Za-z]+\\b");
    rule.format = typeFormat;
    highlightingRules.append(rule);

    propertyFormat.setForeground(COLOR_PROPERTIES);
    rule.pattern = QRegExp("^(!?(\\s+)?)[a-z][\\w\\.]+([ \\t ]+)?(?=:)");
    rule.format = propertyFormat;
    highlightingRules.append(rule);

    classFormat.setForeground(COLOR_QTCLASS);
    rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

    singleLineCommentFormat.setForeground(COLOR_COMMMENTS);
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(COLOR_COMMMENTS);

    quotationFormat.setForeground(COLOR_STRING);
    rule.pattern = QRegExp("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    functionFormat.setFontItalic(true);
    functionFormat.setForeground(COLOR_FUNCTION);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");
}

void QmlHighlighter::highlightBlock(const QString &text)
{
    for (const auto& rule : highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStartExpression.indexIn(text);

    while (startIndex >= 0) {
        int endIndex = commentEndExpression.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                            + commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
    }
}
