#ifndef BEHAVIORSETTINGS_H
#define BEHAVIORSETTINGS_H

#include <settings.h>

class CodeEditorSettings;

struct BehaviorSettings : public Settings
{
    BehaviorSettings(CodeEditorSettings* codeEditorSettings = nullptr);

    void read() override;
    void write() override;
    void reset() override;
    const char* category() const override;

    bool autoSaveBeforeRunning;
};

#endif // BEHAVIORSETTINGS_H