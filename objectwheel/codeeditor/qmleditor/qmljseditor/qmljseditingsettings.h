#pragma once

#include <QtGlobal>

QT_BEGIN_NAMESPACE
class QSettings;
QT_END_NAMESPACE

namespace QmlJSEditor {

class QmlJsEditingSettings {
public:
    QmlJsEditingSettings();

    static QmlJsEditingSettings get();
    void set();

    void fromSettings(QSettings *);
    void toSettings(QSettings *) const;

    bool equals(const QmlJsEditingSettings &other) const;

    bool enableContextPane() const;
    void setEnableContextPane(const bool enableContextPane);

    bool pinContextPane() const;
    void setPinContextPane(const bool pinContextPane);

    bool autoFormatOnSave() const;
    void setAutoFormatOnSave(const bool autoFormatOnSave);

    bool autoFormatOnlyCurrentProject() const;
    void setAutoFormatOnlyCurrentProject(const bool autoFormatOnlyCurrentProject);

private:
    bool m_enableContextPane;
    bool m_pinContextPane;
    bool m_autoFormatOnSave;
    bool m_autoFormatOnlyCurrentProject;
};

inline bool operator==(const QmlJsEditingSettings &s1, const QmlJsEditingSettings &s2)
{ return s1.equals(s2); }
inline bool operator!=(const QmlJsEditingSettings &s1, const QmlJsEditingSettings &s2)
{ return !s1.equals(s2); }

} // namespace QmlJSEditor
