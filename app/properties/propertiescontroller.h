#ifndef PROPERTIESCONTROLLER_H
#define PROPERTIESCONTROLLER_H

#include <renderinfo.h>

class Control;
class PropertiesPane;
class DesignerScene;
class QTreeWidgetItem;

class PropertiesController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PropertiesController)

public:
    explicit PropertiesController(PropertiesPane* propertiesPane, DesignerScene* designerScene,
                                  QObject* parent = nullptr);
public slots:
    void discharge() const;
    void clear() const;

private slots:
    void onSearchEditEditingFinish() const;
    void onControlZChange(Control*) const;
    void onControlRenderInfoChange(Control*, bool codeChanged) const;
    void onControlGeometryChange(const Control*) const;
    void onControlIndexChange(Control*) const;
    void onControlIdChange(Control*, const QString& previousId) const;
    void onControlPropertyChange() const;
    void onControlIdEditingFinish() const;
    void onControlIndexEditingFinish() const;
    void onSceneSelectionChange() const;

    void onIntPropertyEdit(const QString& propertyName, const QVariant& value) const;
    void onRealPropertyEdit(const QString& propertyName, const QVariant& value) const;
    void onFontSizePropertyEdit(QTreeWidgetItem* fontClassItem, const QString& propertyName, const QVariant& value) const;
    void onFontFamilyPropertyEdit(QTreeWidgetItem* fontClassItem, const QVariant& value) const;
    void onFontWeightPropertyEdit(const QMetaEnum& _enum, const QVariant& value) const;
    void onFontCapitalizationPropertyEdit(const QMetaEnum& _enum, const QVariant& value) const;
    void onEnumPropertyEdit(const QString& propertyName, const Enum& _enum, const QVariant& value) const;
    void onUrlPropertyEdit(const QString& propertyName, const QVariant& value) const;
    void onStringPropertyEdit(const QString& propertyName, const QVariant& value) const;
    void onBoolPropertyEdit(const QString& propertyName, const QVariant& value) const;
    void onColorPropertyEdit(const QString& propertyName, const QVariant& value) const;

private:
    Control* control() const;

private:
    PropertiesPane* m_propertiesPane;
    DesignerScene* m_designerScene;
};

#endif // PROPERTIESCONTROLLER_H