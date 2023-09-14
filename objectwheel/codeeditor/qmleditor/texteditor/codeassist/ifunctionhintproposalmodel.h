// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "iassistproposalmodel.h"

QT_FORWARD_DECLARE_CLASS(QString);

namespace TextEditor {

class IFunctionHintProposalModel : public IAssistProposalModel
{
public:
    IFunctionHintProposalModel();
    ~IFunctionHintProposalModel() override;

    virtual int activeArgument(const QString &prefix) const = 0;
    virtual QString id(int index) const;
};

using FunctionHintProposalModelPtr = QSharedPointer<IFunctionHintProposalModel>;

} // TextEditor
