#ifndef MODESELECTORCONTROLLER_H
#define MODESELECTORCONTROLLER_H

#include <modemanager.h>

class ModeSelectorPane;
class ModeSelectorController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ModeSelectorController)

public:
    explicit ModeSelectorController(ModeSelectorPane* m_modeSelectorPane, QObject* parent = nullptr);

public slots:
    void discharge();

private slots:
    void onModeChange(ModeManager::Mode mode);
    void onDesignerActionTriggered(bool checked);
    void onEditorActionTriggered(bool checked);
    void onSplitActionTriggered(bool checked);
    void onOptionsActionTriggered(bool checked);
    void onBuildsActionTriggered(bool checked);
    void onDocumentsActionTriggered(bool checked);

private:
    ModeSelectorPane* m_modeSelectorPane;
};

#endif // MODESELECTORCONTROLLER_H