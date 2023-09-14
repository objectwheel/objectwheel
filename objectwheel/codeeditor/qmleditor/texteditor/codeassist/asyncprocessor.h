// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "iassistprocessor.h"

#include <QFutureWatcher>

namespace TextEditor {

class AsyncProcessor : public TextEditor::IAssistProcessor
{
public:
    AsyncProcessor();

    IAssistProposal *perform() final;
    bool running() override;
    void cancel() override;

    virtual IAssistProposal *performAsync() = 0;
    virtual IAssistProposal *immediateProposal();

protected:
    bool isCanceled() const;

private:
    QFutureWatcher<IAssistProposal *> m_watcher;
};

} // namespace TextEditor
