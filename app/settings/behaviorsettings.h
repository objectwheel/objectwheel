#ifndef BEHAVIORSETTINGS_H
#define BEHAVIORSETTINGS_H

#include <settings.h>

class CodeEditorSettings;
struct BehaviorSettings final : public Settings
{
    explicit BehaviorSettings(CodeEditorSettings* codeEditorSettings);
    void read() override;
    void write() override;
    void reset() override;
    const char* category() const override;

    bool autoSaveBeforeRunning;
};

#endif // BEHAVIORSETTINGS_H