// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include <QSharedPointer>

QT_FORWARD_DECLARE_CLASS(QString)

namespace TextEditor {

class IAssistProposalModel
{
public:
    IAssistProposalModel();
    virtual ~IAssistProposalModel();

    virtual void reset() = 0;
    virtual int size() const = 0;
    virtual QString text(int index) const = 0;
};

using ProposalModelPtr = QSharedPointer<IAssistProposalModel>;

} // TextEditor
