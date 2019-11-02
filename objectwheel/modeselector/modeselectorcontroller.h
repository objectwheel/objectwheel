#ifndef MODESELECTORCONTROLLER_H
#define MODESELECTORCONTROLLER_H

#include <modemanager.h>

class ModeSelectorPane;
class ModeSelectorController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ModeSelectorController)

public:
    explicit ModeSelectorController(ModeSelectorPane* modeSelectorPane, QObject* parent = nullptr);

public slots:
    void discharge();

private slots:
    void onModeChange(ModeManager::Mode mode);
    void onDesignerButtonClick(bool checked);
    void onEditorButtonClick(bool checked);
    void onSplitButtonClick(bool checked);
    void onOptionsButtonClick(bool checked);
    void onBuildsButtonClick(bool checked);
    void onDocumentsButtonClick(bool checked);

private:
    ModeSelectorPane* m_modeSelectorPane;
};

#endif // MODESELECTORCONTROLLER_H