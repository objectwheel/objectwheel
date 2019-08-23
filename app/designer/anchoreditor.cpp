#include <anchoreditor.h>

AnchorEditor::AnchorEditor(QWidget* parent) : QWidget(parent)
{
}

Control* AnchorEditor::sourceControl() const
{
    return m_sourceControl;
}

void AnchorEditor::setSourceControl(Control* sourceControl)
{
    m_sourceControl = sourceControl;
}

Control* AnchorEditor::targetControl() const
{
    return m_targetControl;
}

void AnchorEditor::setTargetControl(Control* targetControl)
{
    m_targetControl = targetControl;
}
