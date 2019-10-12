#ifndef PROPERTIESCONTROLLER_H
#define PROPERTIESCONTROLLER_H

#include <QObject>
#include <QHash>

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
    void discharge();
    void clear() const;

private slots:
    void onResetButtonClick() const;
    void onSearchEditEditingFinish() const;
    void onControlZChange(Control*) const;
    void onControlRenderInfoChange(Control*, bool codeChanged);
    void onControlGeometryChange(const Control*) const;
    void onControlIndexChange(Control*) const;
    void onControlIdChange(Control*, const QString& previousId) const;
    void onControlPropertyChange() const;
    void onControlIdEditingFinish() const;
    void onControlIndexValueChange() const;
    void onSceneSelectionChange();

    void onIntPropertyEdit(QTreeWidgetItem* item, QTreeWidgetItem* classItem,
                           const QString& propertyName, const QVariant& value) const;
    void onRealPropertyEdit(QTreeWidgetItem* item, QTreeWidgetItem* classItem,
                            const QString& propertyName, const QVariant& value) const;
    void onFontSizePropertyEdit(QTreeWidgetItem* item, QTreeWidgetItem* opponentItem,
                                QTreeWidgetItem* classItem, const QString& propertyName,
                                const QVariant& value) const;
    void onFontFamilyPropertyEdit(QTreeWidgetItem* item, QTreeWidgetItem* classItem,
                                  const QVariant& value) const;
    void onFontWeightPropertyEdit(QTreeWidgetItem* item, QTreeWidgetItem* classItem,
                                  const QVariant& value) const;
    void onFontCapitalizationPropertyEdit(QTreeWidgetItem* item, QTreeWidgetItem* fontClassItem,
                                          const QVariant& value) const;
    void onEnumPropertyEdit(QTreeWidgetItem* item, QTreeWidgetItem* classItem,
                            const QString& propertyName, const QVariant& value) const;
    void onUrlPropertyEdit(QTreeWidgetItem* item, QTreeWidgetItem* classItem,
                           const QString& propertyName, const QVariant& value) const;
    void onStringPropertyEdit(QTreeWidgetItem* item, QTreeWidgetItem* classItem,
                              const QString& propertyName, const QVariant& value) const;
    void onBoolPropertyEdit(QTreeWidgetItem* item, QTreeWidgetItem* classItem,
                            const QString& propertyName, const QVariant& value) const;
    void onColorPropertyEdit(QTreeWidgetItem* item, QTreeWidgetItem* classItem,
                             const QString& propertyName, const QVariant& value) const;

private:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    Control* control() const;

private:
    PropertiesPane* m_propertiesPane;
    DesignerScene* m_designerScene;
    QString m_selectedQmlType;
    QHash<QString, int> m_verticalScrollPositions;
    QHash<QString, int> m_horizontalScrollPositions;
    bool m_fontItemOpen;
    bool m_geometryItemOpen;
    bool m_isExpandCollapseSignalsBlocked;
};

#endif // PROPERTIESCONTROLLER_H