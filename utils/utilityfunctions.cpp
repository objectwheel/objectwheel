#include <utilityfunctions.h>
#include <delayer.h>
#include <utils/utilsicons.h>
#include <paintutils.h>
#include <crossplatform.h>
#include <async.h>
#include <filesystemutils.h>

#include <QFileInfo>
#include <QMessageBox>
#include <QQmlEngine>
#include <QTextDocument>
#include <QRegularExpression>
#include <QTextCursor>
#include <QProgressDialog>
#include <QTreeView>
#include <QScreen>
#include <QWindow>
#include <QAbstractButton>
#include <QComboBox>
#include <QApplication>
#include <QAction>
#include <QJsonObject>
#include <QDir>

namespace UtilityFunctions {

namespace Internal {

void pushHelper(QDataStream&) {}
void pullHelper(QDataStream&) {}

} // Internal

void trimCommentsAndStrings(QTextDocument* document)
{
    QRegularExpression exp("(([\"'])(?:\\\\[\\s\\S]|.)*?\\2|\\/(?![*\\/])(?:\\\\.|\\[(?:\\\\.|.)\\]|.)*?\\/)|\\/\\/.*?$|\\/\\*[\\s\\S]*?\\*\\/",
                           QRegularExpression::MultilineOption); // stackoverflow.com/q/24518020

    QRegularExpressionMatchIterator i = exp.globalMatch(document->toPlainText());
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString capturedText = match.captured();
        int start = match.capturedStart();
        int end = match.capturedEnd();

        capturedText.replace(QRegularExpression("[^\\n\\r\\'\\\"]", QRegularExpression::MultilineOption), "x");

        QTextCursor cursor(document);
        cursor.setPosition(start);
        cursor.setPosition(end, QTextCursor::KeepAnchor);
        cursor.insertText(capturedText);
    }
}

QWidget* createSpacingWidget(const QSize& size)
{
    auto spacing = new QWidget;
    spacing->setFixedSize(size);
    return spacing;
}

QWidget* createSpacerWidget(Qt::Orientation orientation)
{
    auto spacer = new QWidget;
    spacer->setSizePolicy((orientation & Qt::Horizontal)
                          ? QSizePolicy::Expanding : QSizePolicy::Preferred,
                          (orientation & Qt::Vertical)
                          ? QSizePolicy::Expanding : QSizePolicy::Preferred);
    return spacer;
}

QWidget* createSeparatorWidget(Qt::Orientation orientation)
{
    auto separator = new QWidget;
    separator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    if (orientation & Qt::Horizontal)
        separator->setFixedHeight(1);
    else
        separator->setFixedWidth(1);
    separator->setStyleSheet("border: none; background: #c4c4c4;");
    return separator;
}

void copyFiles(const QString& rootPath, const QList<QUrl>& urls, QWidget* parent)
{
    QProgressDialog progress("Copying files...", "Abort Copy", 0, urls.size(), parent);
    progress.setWindowModality(Qt::NonModal);
    progress.open();
    Delayer::delay(100);

    bool askForOverwrite = true;

    for (int i = 0; i < urls.size(); i++) {
        const QUrl& url = urls.at(i);

        progress.setValue(i);

        if (progress.wasCanceled())
            break;

        if (url.isEmpty())
            continue;

        if (!url.isValid())
            continue;

        if (!url.isLocalFile())
            continue;

        const QString& path = QFileInfo(url.toLocalFile()).canonicalFilePath();
        const QString& fileName = QFileInfo(path).fileName();
        const QString& destPath = rootPath + '/' + fileName;

        if (QFileInfo::exists(destPath)) {
            if (askForOverwrite) {
                int ret = QMessageBox::question(
                            parent,
                            QObject::tr("File or folder exists"),
                            QObject::tr("File or folder exists. "
                                        "Would you like to overwrite following file/folder: ") + fileName,
                            QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll| QMessageBox::Abort,
                            QMessageBox::No);
                if (ret == QMessageBox::Yes) {
                    if (QFileInfo(destPath).isDir())
                        QDir(destPath).removeRecursively();
                    else
                        QFile::remove(destPath);
                } else if (ret == QMessageBox::No) {
                    continue;
                } else if (ret == QMessageBox::YesToAll) {
                    askForOverwrite = false;
                    if (QFileInfo(destPath).isDir())
                        QDir(destPath).removeRecursively();
                    else
                        QFile::remove(destPath);
                } else {
                    break;
                }
            } else {
                if (QFileInfo(destPath).isDir())
                    QDir(destPath).removeRecursively();
                else
                    QFile::remove(destPath);
            }
        }

        QFuture<void> future = Async::run(QThreadPool::globalInstance(),
                                          &FileSystemUtils::copy,
                                          path, rootPath, false, false);
        Delayer::delay(std::bind(&QFuture<void>::isRunning, &future));
    }

    progress.setValue(urls.size());
    Delayer::delay(100);
}

void expandUpToRoot(QTreeView* view, const QModelIndex& index, const QModelIndex& rootIndex)
{
    if (!index.isValid())
        return;

    if (index == rootIndex)
        return;

    view->expand(index);

    expandUpToRoot(view, index.parent(), rootIndex);
}

QWindow* window(const QWidget* widget)
{
    Q_ASSERT(widget);
    QWindow* winHandle = widget->windowHandle();
    if (!winHandle) {
        if (const QWidget* nativeParent = widget->nativeParentWidget())
            winHandle = nativeParent->windowHandle();
    }
    return winHandle;
}

void centralizeWidget(QWidget* widget)
{
    Q_ASSERT(window(widget));
    widget->setGeometry(QStyle::alignedRect(widget->layoutDirection(), Qt::AlignCenter, widget->size(),
                                            window(widget)->screen()->availableGeometry()));
}

void adjustFontPixelSize(QWidget* widget, int advance)
{
    QFont font(widget->font());
    font.setPixelSize(font.pixelSize() + advance);
    widget->setFont(font);
}

void adjustFontWeight(QWidget* widget, QFont::Weight weight)
{
    QFont font(widget->font());
    font.setWeight(weight);
    widget->setFont(font);
}

bool hasHover(const QWidget* widget) // FIXME: This is a workaround for QTBUG-44400
{
    return widget->isVisible()
            && widget->isEnabled()
            && widget->rect().contains(widget->mapFromGlobal(QCursor::pos()));
}

bool isEmailFormatCorrect(const QString& email)
{
    return email.contains(QRegularExpression("^[a-z0-9._%+-]+@[a-z0-9.-]+\\.[a-z]{2,4}$"));
}

bool isPasswordFormatCorrect(const QString& password)
{
    return password.contains(QRegularExpression("^[><{}\\[\\]*!@\\-#$%^&+=~\\.\\,\\:a-zA-Z0-9]{6,35}$"));
}

QRectF verticalAlignedRect(const QSizeF& size, const QRectF& rect, qreal left)
{
    QRectF ret({0, 0}, size);
    ret.moveCenter(rect.center());
    if (left >= 0)
        ret.moveLeft(left);
    return ret;
}

QRectF horizontalAlignedRect(const QSizeF& size, const QRectF& rect, qreal top)
{
    QRectF ret({0, 0}, size);
    ret.moveCenter(rect.center());
    if (top >= 0)
        ret.moveTop(top);
    return ret;
}

QPixmap pixmap(QAbstractButton* button, const QIcon& icon, const QSizeF& size)
{
    Q_ASSERT(window(button));
    return icon.pixmap(window(button), size.toSize(), !button->isEnabled()
                       ? QIcon::Disabled
                       : (button->isDown() ? QIcon::Active : QIcon::Normal));
}

QPixmap scaled(const QPixmap& pixmap, const QSize& size)
{
    return pixmap.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

QIcon iconForQmlError(const QQmlError& error, const QAbstractItemView* view)
{
    static QIcon info, warning, critical;
    if (info.isNull()) {
        info = Utils::Icons::INFO.icon();
        warning = Utils::Icons::WARNING.icon();
        critical = Utils::Icons::CRITICAL.icon();
        Q_ASSERT(window(view));
        info.addPixmap(info.pixmap(window(view), view->iconSize(), QIcon::Normal), QIcon::Selected);
        warning.addPixmap(warning.pixmap(window(view), view->iconSize(), QIcon::Normal), QIcon::Selected);
        critical.addPixmap(critical.pixmap(window(view), view->iconSize(), QIcon::Normal), QIcon::Selected);
    }

    switch (error.messageType()) {
    case QtInfoMsg:
    case QtDebugMsg:
        return info;
    case QtCriticalMsg:
    case QtFatalMsg:
        return critical;
    case QtWarningMsg: // TODO: Fix this when Qt has a proper fix
        return /*warning*/critical;
    default:
        return info;
    }
}

bool comboContainsWord(QComboBox* comboBox, const QString& word)
{
    for (int i = 0; i < comboBox->count(); ++i) {
        if (comboBox->itemText(i).contains(word, Qt::CaseInsensitive))
            return true;
    }
    return false;
}

QPoint centerPos(const QSize& size)
{
    return QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size,
                               QApplication::primaryScreen()->availableGeometry()).topLeft();
}

QString increasedNumberedText(const QString& text, bool addSpace, bool trim)
{
    int number = 0;
    QString finalText(trim ? text.trimmed() : text);
    bool isLastElementNumber = false;

    if (!finalText.isEmpty())
        number = finalText.right(1).toInt(&isLastElementNumber);

    if (!isLastElementNumber) {
        if (addSpace && !finalText.isEmpty()) {
            if (finalText.back().isSpace())
                return finalText.append("1");
            else
                return finalText.append(" 1");
        } else {
            return finalText.append("1");
        }
    }

    // Empty strings cannot go further

    Q_ASSERT(number >= 0 && number <= 9);

    if (number != 9)
        return finalText.replace(finalText.size() - 1, 1, QString::number(number + 1));

    if (finalText.size() > 1)
        return increasedNumberedText(finalText.left(finalText.size() - 1), false, false) + "0";
    else
        return "10";
}

QRectF getGeometryFromProperties(const QList<PropertyNode>& properties)
{
    QRectF geometry;
    for (const PropertyNode& propertyNode : properties) {
        for (const QString& propertyName : propertyNode.properties.keys()) {
            if (propertyName == "x")
                geometry.moveLeft(propertyNode.properties.value(propertyName).toReal());
            else if (propertyName == "y")
                geometry.moveTop(propertyNode.properties.value(propertyName).toReal());
            else if (propertyName == "width")
                geometry.setWidth(propertyNode.properties.value(propertyName).toReal());
            else if (propertyName == "height")
                geometry.setHeight(propertyNode.properties.value(propertyName).toReal());
        }
    }
    return geometry;
}

QMarginsF getMarginsFromProperties(const QList<PropertyNode>& properties)
{
    QMarginsF margins;
    for (const PropertyNode& propertyNode : properties) {
        for (const QString& propertyName : propertyNode.properties.keys()) {
            if (propertyName == "__ow_margins_left")
                margins.setLeft(propertyNode.properties.value(propertyName).toReal());
            else if (propertyName == "__ow_margins_top")
                margins.setTop(propertyNode.properties.value(propertyName).toReal());
            else if (propertyName == "__ow_margins_right")
                margins.setRight(propertyNode.properties.value(propertyName).toReal());
            else if (propertyName == "__ow_margins_bottom")
                margins.setBottom(propertyNode.properties.value(propertyName).toReal());
        }
    }
    return margins;
}

void putMarginsToProperties(QMap<QString, QVariant>& properties, const QMarginsF& margins)
{
    properties["__ow_margins_left"] = margins.left();
    properties["__ow_margins_top"] = margins.top();
    properties["__ow_margins_right"] = margins.right();
    properties["__ow_margins_bottom"] = margins.bottom();
}

QVariant getProperty(const QString& property, const QList<PropertyNode>& properties)
{
    for (const PropertyNode& propertyNode : properties) {
        for (const QString& propertyName : propertyNode.properties.keys()) {
            if (propertyName == property)
                return propertyNode.properties.value(propertyName);
        }
    }
    return QVariant();
}

Enum getEnum(const QString& name, const QList<PropertyNode>& properties)
{
    for (const PropertyNode& propertyNode : properties) {
        for (const Enum& enumm : propertyNode.enums) {
            if (enumm.name == name)
                return enumm;
        }
    }
    return Enum();
}

QVariantMap localDeviceInfo()
{
    static const QJsonObject info = {
        {"currentCpuArchitecture", QSysInfo::currentCpuArchitecture()},
        {"kernelType", QSysInfo::kernelType()},
        {"kernelVersion", QSysInfo::kernelVersion()},
        {"prettyProductName", QSysInfo::prettyProductName()},
        {"productType", QSysInfo::productType()},
        {"deviceName", CrossPlatform::deviceName()},
        {"deviceUid", "000000000000"},
        {"isEmulator", false},
        {"interpreterVersion", "1.2"}
    };
    return info.toVariantMap();
}

QIcon deviceIcon(const QVariantMap& deviceInfo)
{
    const QString productType = deviceInfo.value("productType").toString();
    const QString deviceName = deviceInfo.value("deviceName").toString();
    const QString deviceUid = deviceInfo.value("deviceUid").toString();

    if (deviceUid == localDeviceInfo().value("deviceUid"))
        return PaintUtils::renderButtonIcon(":/images/mycomputer.png");
    if (productType == "ios") {
        if (deviceName.contains("ipad", Qt::CaseInsensitive))
            return PaintUtils::renderButtonIcon(":/images/ipad.svg");
        return PaintUtils::renderButtonIcon(":/images/ios.svg");
    }
    if (productType == "android")
        return PaintUtils::renderButtonIcon(":/images/android.svg");
    if (productType == "osx") {
        if (deviceName.contains("macbook", Qt::CaseInsensitive))
            return PaintUtils::renderButtonIcon(":/images/macbook.svg");
        return PaintUtils::renderButtonIcon(":/images/imac.svg");
    }
    return PaintUtils::renderButtonIcon(":/images/mycomputer.png");
}

QString deviceUid(const QVariantMap& deviceInfo)
{
    return deviceInfo.value("deviceUid").toString();
}

QString deviceName(const QVariantMap& deviceInfo)
{
    const bool isEmulator = deviceInfo.value("isEmulator").toBool();
    const QString deviceName = deviceInfo.value("deviceName").toString();
    if (isEmulator)
        return deviceName + " (Emulator)";
    return deviceName;
}

QString deviceInfoToolTip(const QVariantMap& deviceInfo)
{
    return QString(
    R"(
      <html><body><table>
        <tr style='white-space:pre'><th><img src=":/images/info.png" width="16"/></th><th>%1</th><th></th></tr>
        <tr style='white-space:pre'><td></td><td>%2</td><td>: %3</td></tr>
        <tr style='white-space:pre'><td></td><td>%4</td><td>: %5</td></tr>
        <tr style='white-space:pre'><td></td><td>%6</td><td>: %7</td></tr>
        <tr style='white-space:pre'><td></td><td>%8</td><td>: %9</td></tr>
        <tr style='white-space:pre'><td></td><td>%10</td><td>: %11</td></tr>
        <tr style='white-space:pre'><td></td><td>%12</td><td>: %13</td></tr>
      </table></body></html>
    )")
    .arg(QObject::tr("Device Information"))
    .arg(QObject::tr("Name")).arg(deviceName(deviceInfo))
    .arg(QObject::tr("Unique ID")).arg(deviceInfo["deviceUid"].toString())
    .arg(QObject::tr("Operating System")).arg(deviceInfo["prettyProductName"].toString())
    .arg(QObject::tr("Kernel Type")).arg(deviceInfo["kernelType"].toString())
    .arg(QObject::tr("Kernel Version")).arg(deviceInfo["kernelVersion"].toString())
    .arg(QObject::tr("Cpu Architecture")).arg(deviceInfo["currentCpuArchitecture"].toString());
}

QString deviceUid(const QAction* action)
{
    return action->property("__OW_DEVICE_INFO__").value<QVariantMap>().value("deviceUid").toString();
}

void setDeviceInfo(QAction* action, const QVariantMap& deviceInfo)
{
    action->setProperty("__OW_DEVICE_INFO__", deviceInfo);
}

QString toPrettyBytesString(qint64 bytes, bool withExt)
{
    QString ret;
    float kb = 1024.0f;
    float mb = 1048576.0f;
    float gb = 1073741824.0f;

    if (bytes < kb) {
        ret = QString::number(bytes);
        if (withExt)
            ret += " Bytes";
    } else if (bytes < mb) {
        ret = QString::number(bytes / kb, 'f', 1);
        if (withExt)
            ret += " Kb";
    } else if (bytes < gb) {
        ret = QString::number(bytes / mb, 'f', 1);
        if (withExt)
            ret += " Mb";
    } else {
        ret = QString::number(bytes / gb, 'f', 2);
        if (withExt)
            ret += " Gb";
    }

    return ret;
}

} // UtilityFunctions