#ifndef ANDROIDPLATFORMCONTROLLER_H
#define ANDROIDPLATFORMCONTROLLER_H

#include <abstractplatformcontroller.h>

class AndroidPlatformWidget;
class AndroidPlatformController final : public AbstractPlatformController
{
    Q_OBJECT
    Q_DISABLE_COPY(AndroidPlatformController)

public:
    explicit AndroidPlatformController(AndroidPlatformWidget* androidPlatformWidget,
                                       QObject* parent = nullptr);
    bool isComplete() const override;

    QCborMap toCborMap() const override;

public slots:
    void charge() const;

private slots:
    void onNameEdit(QString name) const;
    void onDomainEdit(QString domain) const;
    void onPackageEdit(const QString& package) const;
    void onVersionSpinValueChange(int value) const;
    void onBrowseIconButtonClick() const;
    void onClearIconButtonClick() const;
    void onIncludePemissionsCheckClick(bool checked) const;
    void onAddPermissionButtonClick() const;
    void onRemovePermissionButtonClick() const;
    void onPermissionListItemSelectionChange() const;
    void onIncludeQtModulesCheckClick(bool checked) const;
    void onAddQtModuleButtonClick() const;
    void onRemoveQtModuleButtonClick() const;
    void onQtModuleListItemSelectionChange() const;
    void onAbiCheckClick();

private:
    QString generatePackageName(const QString& rawDomain, const QString& rawAppName) const;

private:
    AndroidPlatformWidget* m_androidPlatformWidget;
};

#endif // ANDROIDPLATFORMCONTROLLER_H
