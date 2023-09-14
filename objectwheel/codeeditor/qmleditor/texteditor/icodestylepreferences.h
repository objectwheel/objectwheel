// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include <QObject>

class QVariant;
class QSettings;

namespace TextEditor {

namespace Internal { class ICodeStylePreferencesPrivate; }

class TabSettings;
class CodeStylePool;

class ICodeStylePreferences : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
public:
    // pool is a pool which will be used by this preferences for setting delegates
    explicit ICodeStylePreferences(QObject *parentObject = nullptr);
    ~ICodeStylePreferences() override;

    QByteArray id() const;
    void setId(const QByteArray &name);

    QString displayName() const;
    void setDisplayName(const QString &name);

    bool isReadOnly() const;
    void setReadOnly(bool on);

    bool isTemporarilyReadOnly() const;
    void setTemporarilyReadOnly(bool on);

    bool isAdditionalTabDisabled() const;
    void setIsAdditionalTabDisabled(bool on);

    void setTabSettings(const TabSettings &settings);
    TabSettings tabSettings() const;
    TabSettings currentTabSettings() const;

    virtual QVariant value() const = 0;
    virtual void setValue(const QVariant &) = 0;

    QVariant currentValue() const; // may be from grandparent

    ICodeStylePreferences *currentPreferences() const; // may be grandparent

    CodeStylePool *delegatingPool() const;
    void setDelegatingPool(CodeStylePool *pool);

    ICodeStylePreferences *currentDelegate() const; // null or one of delegates from the pool
    void setCurrentDelegate(ICodeStylePreferences *delegate);

    QByteArray currentDelegateId() const;
    void setCurrentDelegate(const QByteArray &id);

    void setSettingsSuffix(const QString &suffix);
    void toSettings(const QString &category, QSettings *s) const;
    void fromSettings(const QString &category, QSettings *s);

    // make below 2 protected?
    virtual QVariantMap toMap() const;
    virtual void fromMap(const QVariantMap &map);

signals:
    void tabSettingsChanged(const TextEditor::TabSettings &settings);
    void currentTabSettingsChanged(const TextEditor::TabSettings &settings);
    void valueChanged(const QVariant &);
    void currentValueChanged(const QVariant &);
    void currentDelegateChanged(TextEditor::ICodeStylePreferences *currentDelegate);
    void currentPreferencesChanged(TextEditor::ICodeStylePreferences *currentPreferences);
    void displayNameChanged(const QString &newName);

private:
    void codeStyleRemoved(ICodeStylePreferences *preferences);

    Internal::ICodeStylePreferencesPrivate *d;
};


} // namespace TextEditor
