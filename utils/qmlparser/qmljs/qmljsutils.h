// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "parser/qmljsastfwd_p.h"
#include "parser/qmljsdiagnosticmessage_p.h"
#include "parser/qmljsengine_p.h"
#include <utils_global.h>
#include "qmljsconstants.h"
#include <utils/filepath.h>

QT_FORWARD_DECLARE_CLASS(QColor)

namespace QmlJS {

UTILS_EXPORT QColor toQColor(const QString &qmlColorString);
UTILS_EXPORT QString toString(AST::UiQualifiedId *qualifiedId,
                              const QChar delimiter = QLatin1Char('.'));

UTILS_EXPORT SourceLocation locationFromRange(const SourceLocation &start,
                                                   const SourceLocation &end);

UTILS_EXPORT SourceLocation fullLocationForQualifiedId(AST::UiQualifiedId *);

UTILS_EXPORT QString idOfObject(AST::Node *object, AST::UiScriptBinding **idBinding = nullptr);

UTILS_EXPORT AST::UiObjectInitializer *initializerOfObject(AST::Node *object);

UTILS_EXPORT AST::UiQualifiedId *qualifiedTypeNameId(AST::Node *node);

UTILS_EXPORT bool isValidBuiltinPropertyType(const QString &name);

UTILS_EXPORT DiagnosticMessage errorMessage(const SourceLocation &loc,
                                            const QString &message);

UTILS_EXPORT bool maybeModuleVersion(const QString &version);

UTILS_EXPORT const QStringList splitVersion(const QString &version);
UTILS_EXPORT QList<Utils::FilePath> modulePaths(const QString &moduleImportName,
                                                const QString &version,
                                                const QList<Utils::FilePath> &importPaths);

template <class T>
SourceLocation locationFromRange(const T *node)
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
