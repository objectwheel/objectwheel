// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <utils_global.h>
#include <QtCore/qstring.h>
#include <languageutils/componentversion.h>

UTILS_EXPORT QStringList qQmlResolveImportPaths(QStringView uri, const QStringList &basePaths,
                                   LanguageUtils::ComponentVersion version);
