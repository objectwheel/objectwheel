#ifndef PROPERTIESPANE_H
#define PROPERTIESPANE_H

#include <QWidget>

class QVBoxLayout;
class QTreeWidget;
class FocuslessLineEdit;
class DesignerScene;
class Control;
class QTreeWidgetItem;

class PropertiesPane : public QWidget
{
		Q_OBJECT

    public:
        enum NodeType {
            EnumType,
            FontFamily,
            FontWeight,
            FontPtSize,
            FontPxSize,
            FontBold,
            FontItalic,
            FontUnderline,
            FontOverline,
            FontStrikeout,
            Color,
            Bool,
            String,
            Id,
            Url,
            Double,
            Int,
            GeometryX,
            GeometryY,
            GeometryWidth,
            GeometryHeight,
            GeometryFX,
            GeometryFY,
            GeometryFWidth,
            GeometryFHeight
        };

        enum NodeRole {
            Type = Qt::UserRole + 1,
            Data
        };

	public:
        explicit PropertiesPane(DesignerScene* designerScene, QWidget* parent = nullptr);
        void saveChanges(const QString& property, const QString& parserValue, const QVariant& value);
        void saveChanges(const NodeType& type, const QString& parserValue, const QVariant& value);

	public slots:
        void reset();
        void refreshList();

    protected:
        QSize sizeHint() const override;
        bool eventFilter(QObject *watched, QEvent *event) override;

    private slots:
        void clearList();
        void handleSelectionChange();
        void filterList(const QString& filter);

    private:
        void processFont(QTreeWidgetItem* item, const QString& propertyName, const QMap<QString, QVariant>& map);
        void processGeometry(QTreeWidgetItem* item, const QString& propertyName, Control* control);
        void processGeometryF(QTreeWidgetItem* item, const QString& propertyName, Control* control);
        void processColor(QTreeWidgetItem* item, const QString& propertyName, const QMap<QString, QVariant>& map);
        void processBool(QTreeWidgetItem* item, const QString& propertyName, const QMap<QString, QVariant>& map);
        void processString(QTreeWidgetItem* item, const QString& propertyName, const QMap<QString, QVariant>& map);
        void processUrl(QTreeWidgetItem* item, const QString& propertyName, const QMap<QString, QVariant>& map);
        void processDouble(QTreeWidgetItem* item, const QString& propertyName, const QMap<QString, QVariant>& map);
        void processInt(QTreeWidgetItem* item, const QString& propertyName, const QMap<QString, QVariant>& map);

    public:
        DesignerScene* m_designerScene;

    private:
        QVBoxLayout* _layout;
        QTreeWidget* _treeWidget;
        FocuslessLineEdit* _searchEdit;
};

Q_DECLARE_METATYPE(PropertiesPane::NodeType)

#endif // PROPERTIESPANE_H
