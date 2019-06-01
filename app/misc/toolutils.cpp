#include <toolutils.h>
#include <saveutils.h>
#include <parserutils.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QPixmap>

namespace ToolUtils {

namespace Internal {

const QJsonObject& toolIcons()
{
    static QJsonObject icons;
    if (icons.isEmpty()) {
        QFile file(":/resources/other/tool-icons.json");
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
        QFile file(":/resources/other/tool-categories.json");
        if (!file.open(QFile::ReadOnly)) {
            qWarning("ToolUtils: Failed to read reference json file for tool categories");
            return categories;
        }
        categories = QJsonDocument::fromJson(file.readAll()).object();
        file.close();
    }
    return categories;
}

} // Internal

QPixmap toolIcon(const QString& controlDir, double dpr)
{
    Q_ASSERT(SaveUtils::isControlValid(controlDir));

    const QJsonObject& icons(Internal::toolIcons());
    const QString& moduleName = ParserUtils::moduleName(controlDir);

    QPixmap icon(icons.value(moduleName).toString());
    icon.setDevicePixelRatio(dpr);
    return icon;
}

QString toolName(const QString& controlDir)
{
    Q_ASSERT(SaveUtils::isControlValid(controlDir));

    const QString& moduleName = ParserUtils::moduleName(controlDir);
    const QStringList& pieces = moduleName.split('.');

    QString name(QObject::tr("Tool"));
    if (pieces.size() > 1)
        name = pieces.last();

    return name;
}

QString toolCetegory(const QString& controlDir)
{
    Q_ASSERT(SaveUtils::isControlValid(controlDir));

    const QJsonObject& categories(Internal::toolCategories());
    const QString& moduleName = ParserUtils::moduleName(controlDir);
    QStringList pieces = moduleName.split('.');

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

} // ToolUtils