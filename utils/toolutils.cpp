#include <toolutils.h>
#include <saveutils.h>
#include <parserutils.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QPixmap>

namespace ToolUtils {

namespace Internal {

QString majorModule(const QString& module)
{
    QString majorModule = module;
    int majorVersion = ParserUtils::moduleVersionMajor(module);
    if (majorVersion >= 0)
        majorModule = ParserUtils::moduleBody(module) + QStringLiteral("/") + QString::number(majorVersion);
    return majorModule;
}

const QJsonObject& toolIcons()
{
    static QJsonObject icons;
    if (icons.isEmpty()) {
        QFile file(":/other/tool-icons.json");
        if (!file.open(QFile::ReadOnly)) {
            qWarning("ToolUtils: Failed to read reference json file for tool icons");
            return icons;
        }
        icons = QJsonDocument::fromJson(file.readAll()).object();
        file.close();
    }
    return icons;
}

const QJsonObject& toolCategories()
{
    static QJsonObject categories;
    if (categories.isEmpty()) {
        QFile file(":/other/tool-categories.json");
        if (!file.open(QFile::ReadOnly)) {
            qWarning("ToolUtils: Failed to read reference json file for tool categories");
            return categories;
        }
        categories = QJsonDocument::fromJson(file.readAll()).object().value("categories").toObject();
        file.close();
    }
    return categories;
}

const QList<QString>& toolCategoriesOrder()
{
    static QList<QString> order;
    if (order.isEmpty()) {
        QFile file(":/other/tool-categories.json");
        if (!file.open(QFile::ReadOnly)) {
            qWarning("ToolUtils: Failed to read reference json file for tool order");
            return order;
        }
        for (const QJsonValue& v : QJsonDocument::fromJson(file.readAll()).object().value("order").toArray())
            order.append(v.toString());
        file.close();
    }
    return order;
}
} // Internal

QString toolIconPath(const QString& module)
{
    const QString& majorModule = ParserUtils::moduleToMajorModule(module);
    const QJsonObject& icons(Internal::toolIcons());
    const QString& iconPath = icons.contains(majorModule)
            ? icons.value(majorModule).toString()
            : QStringLiteral(":/images/tools/Unknown.svg");
    return iconPath;
}

QString toolCetegory(const QString& module)
{
    const QString& majorModule = Internal::majorModule(module);
    const QJsonObject& categories(Internal::toolCategories());
    QString category(QObject::tr("Others"));

    if (categories.contains(majorModule))
        category = categories.value(majorModule).toString();

    return category;
}

int toolCetegoryIndex(const QString& category)
{
    return Internal::toolCategoriesOrder().indexOf(category);
}

} // ToolUtils