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

QString toolName(const QString& controlDir)
{
    Q_ASSERT(SaveUtils::isControlValid(controlDir));
    return toolNameFromUrl(SaveUtils::toControlMainQmlFile(controlDir));
}

QString toolNameFromUrl(const QString& url)
{
    const QString& module = ParserUtils::moduleFromUrl(url);
    const QStringList& pieces = module.split('.');

    QString name(QObject::tr("Tool"));
    if (pieces.size() > 1)
        name = pieces.last();

    return name;
}

QString toolIconPath(const QString& controlDir)
{
    Q_ASSERT(SaveUtils::isControlValid(controlDir));

    const QJsonObject& icons(Internal::toolIcons());
    const QString& module = ParserUtils::module(controlDir);
    const QString& iconPath = icons.contains(module)
            ? icons.value(module).toString()
            : QStringLiteral(":/images/tools/Unknown.svg");
    return iconPath;
}

QString toolCetegory(const QString& controlDir)
{
    Q_ASSERT(SaveUtils::isControlValid(controlDir));

    const QJsonObject& categories(Internal::toolCategories());
    const QString& module = ParserUtils::module(controlDir);
    QStringList pieces = module.split('.');

    QString category(QObject::tr("Others"));
    QString library;
    if (pieces.size() > 1) {
        pieces.removeLast();
        library = pieces.join('.');
    }

    if (categories.contains(library))
        category = categories.value(library).toString();

    return category;
}

int toolCetegoryIndex(const QString& category)
{
    return Internal::toolCategoriesOrder().indexOf(category);
}

} // ToolUtils