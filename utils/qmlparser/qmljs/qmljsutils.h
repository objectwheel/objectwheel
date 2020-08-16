/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#pragma once

#include <utils_global.h>
#include "qmljsconstants.h"
#include "parser/qmljsastfwd_p.h"
#include "parser/qmljsengine_p.h"

QT_FORWARD_DECLARE_CLASS(QColor)

namespace QmlJS {

UTILS_EXPORT QColor toQColor(const QString &qmlColorString);
UTILS_EXPORT QString toString(AST::UiQualifiedId *qualifiedId,
                              const QChar delimiter = QLatin1Char('.'));

UTILS_EXPORT AST::SourceLocation locationFromRange(const AST::SourceLocation &start,
                                                   const AST::SourceLocation &end);

UTILS_EXPORT AST::SourceLocation fullLocationForQualifiedId(AST::UiQualifiedId *);

UTILS_EXPORT QString idOfObject(AST::Node *object, AST::UiScriptBinding **idBinding = 0);

UTILS_EXPORT AST::UiObjectInitializer *initializerOfObject(AST::Node *object);

UTILS_EXPORT AST::UiQualifiedId *qualifiedTypeNameId(AST::Node *node);

UTILS_EXPORT bool isValidBuiltinPropertyType(const QString &name);

UTILS_EXPORT DiagnosticMessage errorMessage(const AST::SourceLocation &loc,
                                            const QString &message);

UTILS_EXPORT bool maybeModuleVersion(const QString &version);

UTILS_EXPORT QString modulePath(const QString &moduleImportName, const QString &version,
                                const QStringList &importPaths);

template <class T>
AST::SourceLocation locationFromRange(const T *node)
{
    return locationFromRange(node->firstSourceLocation(), node->lastSourceLocation());
}

template <class T>
DiagnosticMessage errorMessage(const T *node, const QString &message)
{
    return DiagnosticMessage(QmlJS::Severity::Error,
                             locationFromRange(node),
                             message);
}

} // namespace QmlJS
