#ifndef PROPERTIESCONTROLLER_H
#define PROPERTIESCONTROLLER_H

#include <QObject>

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

private slots:
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

    void onStringPropertyEditingFinish();
    void onBoolPropertyEditingFinish();

private:
    Control* control() const;

private:
    PropertiesPane* m_propertiesPane;
    DesignerScene* m_designerScene;
};

#endif // PROPERTIESCONTROLLER_H