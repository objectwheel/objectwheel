#ifndef PROPERTIESCONTROLLER_H
#define PROPERTIESCONTROLLER_H

#include <renderinfo.h>

class Control;
class PropertiesPane;
class DesignerScene;

class PropertiesController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PropertiesController)

public:
    explicit PropertiesController(PropertiesPane* propertiesPane, DesignerScene* designerScene,
                                  QObject* parent = nullptr);

public slots:
    void discharge();
    void clear();

public slots:
    void onSearchEditEditingFinish();
    void onSceneSelectionChange();
    void onControlZChange(Control*);
    void onControlRenderInfoChange(Control*, bool codeChanged);
    void onControlGeometryChange(const Control*);
    void onControlIndexChange(Control*);
    void onControlIdChange(Control*, const QString& previousId);
    void onControlPropertyChange();
    void onControlIdEditingFinish();
    void onControlIndexEditingFinish();

    void onColorPropertyEdit(const QString& propertyName, const QVariant& value);
    void onStringPropertyEdit(const QString& propertyName, const QVariant& value);
    void onBoolPropertyEdit(const QString& propertyName, const QVariant& value);
    void onEnumPropertyEdit(const QString& propertyName, const Enum& _enum, const QVariant& value);

private:
    Control* control() const;

private:
    PropertiesPane* m_propertiesPane;
    DesignerScene* m_designerScene;
};

#endif // PROPERTIESCONTROLLER_H