// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qmljsastvisitor_p.h"

namespace QmlJS { namespace AST {

Visitor::Visitor(quint16 parentRecursionDepth) : BaseVisitor(parentRecursionDepth)
{
}

BaseVisitor::BaseVisitor(quint16 parentRecursionDepth) : m_recursionDepth(parentRecursionDepth) {}

BaseVisitor::~BaseVisitor()
{
}

}
} // namespace QmlJS::AST
