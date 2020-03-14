#include <utilityfunctions.h>
#include <delayer.h>
#include <paintutils.h>
#include <crossplatform.h>
#include <async.h>
#include <filesystemutils.h>
#include <hashfactory.h>
#include <cmath>

#include <QFileInfo>
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
#include <QCryptographicHash>
#include <QOperatingSystemVersion>
#include <qpassworddigestor.h>
#include <QKeyEvent>
#include <QToolTip>

namespace UtilityFunctions {

namespace Internal {

const char showFocusRingProperty[] = "_q_Objectwheel_showFocusRing";

void pushCborHelper(QCborArray&) {}
void pullCborHelper(QCborArray&) {}

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
    spacing->setAttribute(Qt::WA_TransparentForMouseEvents);
    spacing->setFixedSize(size);
    return spacing;
}

QWidget* createSpacerWidget(Qt::Orientation orientation)
{
    auto spacer = new QWidget;
    spacer->setAttribute(Qt::WA_TransparentForMouseEvents);
    spacer->setSizePolicy((orientation & Qt::Horizontal)
                          ? QSizePolicy::Expanding : QSizePolicy::Preferred,
                          (orientation & Qt::Vertical)
                          ? QSizePolicy::Expanding : QSizePolicy::Preferred);
    return spacer;
}

QWidget* createSeparatorWidget(Qt::Orientation orientation)
{
    auto separator = new QWidget;
    separator->setAttribute(Qt::WA_TransparentForMouseEvents);
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
                int ret = UtilityFunctions::showMessage(
                            parent, QObject::tr("File or folder exists"),
                            QObject::tr("File or folder exists. "
                                        "Would you like to overwrite following file/folder: ") + fileName,
                            QMessageBox::Question,
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

bool isPasswordHashFormatCorrect(const QString& hash)
{
    return hash.contains(QRegularExpression(QStringLiteral("^[a-fA-F0-9]{128}$")));
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

QString toPrettyBytesString(qint64 bytes)
{
    #define KB 1024.0
    #define MB 1048576.0
    #define GB 1073741824.0
    QString ret;
    if (bytes < KB) {
        ret = QString::number(bytes);
        ret += QStringLiteral(" B");
    } else if (bytes < MB) {
        ret = QString::number(bytes / KB, 'f', 2);
        ret += QStringLiteral(" KB");
    } else if (bytes < GB) {
        ret = QString::number(bytes / MB, 'f', 2);
        ret += QStringLiteral(" MB");
    } else {
        ret = QString::number(bytes / GB, 'f', 2);
        ret += QStringLiteral(" GB");
    }
    return ret;
}

QRectF itemGeometry(const QVector<PropertyNode>& properties)
{
    QRectF geometry;
    for (const PropertyNode& propertyNode : properties) {
        const QList<QString>& propertyKeys = propertyNode.properties.keys();
        for (const QString& propertyName : propertyKeys) {
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

QVariant itemProperty(const QString& propertyName, const QVector<PropertyNode>& properties)
{
    for (const PropertyNode& propertyNode : properties) {
        const QList<QString>& propertyKeys = propertyNode.properties.keys();
        for (const QString& name : propertyKeys) {
            if (name == propertyName)
                return propertyNode.properties.value(name);
        }
        for (const Enum& _enum : qAsConst(propertyNode.enums)) {
            if (_enum.name == propertyName)
                return QVariant::fromValue<Enum>(_enum);
        }
    }
    return QVariant();
}

QVariantMap localDeviceInfo()
{
    static const QJsonObject info = {
        {QStringLiteral("currentCpuArchitecture"), QSysInfo::currentCpuArchitecture()},
        {QStringLiteral("kernelType"), QSysInfo::kernelType()},
        {QStringLiteral("kernelVersion"), QSysInfo::kernelVersion()},
        {QStringLiteral("prettyProductName"), QSysInfo::prettyProductName()},
        {QStringLiteral("productType"), QSysInfo::productType()},
        {QStringLiteral("deviceName"), CrossPlatform::deviceName()},
        {QStringLiteral("deviceUid"), QLatin1String("000000000000")},
        {QStringLiteral("version"), QLatin1String(APP_VER)},
        {QStringLiteral("isEmulator"), false}
    };
    return info.toVariantMap();
}

QIcon deviceIcon(const QVariantMap& deviceInfo)
{
    const QString productType = deviceInfo.value("productType").toString();
    const QString deviceName = deviceInfo.value("deviceName").toString();
    const QString deviceUid = deviceInfo.value("deviceUid").toString();

    if (deviceUid == localDeviceInfo().value("deviceUid"))
        return QIcon(":/images/run/computer.svg");
    if (productType == "ios") {
        if (deviceName.contains("ipad", Qt::CaseInsensitive))
            return QIcon(":/images/run/ipad.svg");
        return QIcon(":/images/run/ios.svg");
    }
    if (productType == "android")
        return QIcon(":/images/run/android.svg");
    if (productType == "osx") {
        if (deviceName.contains("macbook", Qt::CaseInsensitive))
            return QIcon(":/images/run/macbook.svg");
        return QIcon(":/images/run/imac.svg");
    }
    return QIcon(":/images/run/computer.svg");
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
    return QString(R"(
                   <html><body><table>
                   <tr style='white-space:pre'><th><img src=':/images/output/info.svg' width='16'/></th><th style='font-weight:500;'>%1</th><th></th></tr>
                   <tr style='white-space:pre'><td></td><td>%2</td><td>: %3</td></tr>
                   <tr style='white-space:pre'><td></td><td>%4</td><td>: %5</td></tr>
                   <tr style='white-space:pre'><td></td><td>%6</td><td>: %7</td></tr>
                   <tr style='white-space:pre'><td></td><td>%8</td><td>: %9</td></tr>
                   <tr style='white-space:pre'><td></td><td>%10</td><td>: %11</td></tr>
                   <tr style='white-space:pre'><td></td><td>%12</td><td>: %13</td></tr>
                   </table></body></html>)")
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

QMessageBox::StandardButton showMessage(QWidget* parent, const QString& title, const QString& text,
                                        QMessageBox::Icon icon, QMessageBox::StandardButtons buttons,
                                        QMessageBox::StandardButton defaultButton, bool modal)
{
    QMessageBox dialog(parent);
    dialog.setIcon(icon);
    dialog.setModal(modal);
    dialog.setStandardButtons(buttons);
    dialog.setDefaultButton(defaultButton);
#if !defined(Q_OS_MACOS)
    dialog.QWidget::setWindowTitle(title);
    dialog.setText(text);
#else
    dialog.setText(title);
    dialog.setInformativeText(text);
    for (QAbstractButton* button : dialog.buttons())
        button->setCursor(Qt::PointingHandCursor);
    if (auto label = dialog.findChild<QWidget*>(QStringLiteral("qt_msgbox_label"))) {
        int MIN_WIDTH = qMax(label->fontMetrics().horizontalAdvance(title), 300);
        label->setStyleSheet(QStringLiteral("QLabel { min-width: %1px; }").arg(MIN_WIDTH));
    }
#endif
    return static_cast<QMessageBox::StandardButton>(dialog.exec());
}

QByteArray generatePasswordHash(const QByteArray& password)
{
    static const QCryptographicHash::Algorithm algorithm = QCryptographicHash::Sha3_512;
    static const quint32 dkLen = QCryptographicHash::hashLength(algorithm);
    static const quint32 iterations = 20000;
    const QByteArray& salt = HashFactory::generateSalt();
    const QByteArray& deriveredKey = QPasswordDigestor::deriveKeyPbkdf2(
                algorithm, password, salt, iterations, dkLen).toHex();
    return push(iterations, salt, deriveredKey);
}

bool testPassword(const QByteArray& password, const QByteArray& hash)
{
    static const QCryptographicHash::Algorithm algorithm = QCryptographicHash::Sha3_512;
    static const quint32 dkLen = QCryptographicHash::hashLength(algorithm);
    quint32 iterations;
    QByteArray salt;
    QByteArray deriveredKey;
    pull(hash, iterations, salt, deriveredKey);
    return !deriveredKey.isEmpty() && deriveredKey == QPasswordDigestor::deriveKeyPbkdf2(
                algorithm, password, salt, iterations, dkLen).toHex();
}

QByteArray generateAutoLoginHash(const QString& email, const QString& pwhash)
{
    static const QByteArray& autoLoginSignature = QByteArrayLiteral("TWHFn2FsbGFo");
    return push(autoLoginSignature, email, pwhash);
}

bool testAutoLogin(const QByteArray& hash, QString* email, QString* pwhash)
{
    QString em, pw;
    QByteArray autoLoginSignature;
    pull(hash, autoLoginSignature, em, pw);
    if (autoLoginSignature == QByteArrayLiteral("TWHFn2FsbGFo")) {
        if (pwhash)
            *pwhash = pw;
        if (email)
            *email = em;
        return true;
    }
    return false;
}

void cleanSensitiveInformation(QString& message)
{
    for (int i = 0; i < message.size(); ++i)
        message[i] = 'x';
    message.clear();
}

QByteArray generateJunk(int sizeInBytes)
{
    QByteArray junk;
    for (int i = 0; i < sizeInBytes; ++i)
        junk.append('x');
    return junk;
}

QString stringify(const QString& text)
{
    QJSEngine engine;
    engine.globalObject().setProperty("text", text);
    return engine.evaluate("JSON.stringify(text)").toString();
}

QStringList zoomTexts()
{
    static const QStringList texts {
        "10 %", "25 %", "50 %", "75 %", "90 %", "100 %", "125 %",
        "150 %", "175 %", "200 %", "300 %", "500 %", "1000 %"
    };
    return texts;
}

QVector<qreal> zoomLevels()
{
    static const QVector<qreal> levels {
        0.1, 0.25, 0.5, 0.75, 0.9, 1.0, 1.25,
        1.5, 1.75, 2.0, 3.0, 5.0, 10.0
    };
    return levels;
}

QString zoomLevelToText(qreal level)
{
    for (int i = 0; i < zoomLevels().size(); ++i) {
        if (zoomLevels().at(i) == level)
            return zoomTexts().at(i);
    }
    return QStringLiteral("100 %");
}

qreal textToZoomLevel(const QString& text)
{
    for (int i = 0; i < zoomTexts().size(); ++i) {
        if (zoomTexts().at(i) == text)
            return zoomLevels().at(i);
    }
    return 1.0;
}

qreal roundZoomLevel(qreal level)
{
    if (level < 0.1)
        return 0.1;
    else if (level >= 0.1 && level < 0.25)
        return 0.1;
    else if (level >= 0.25 && level < 0.5)
        return 0.25;
    else if (level >= 0.5 && level < 0.75)
        return 0.5;
    else if (level >= 0.75 && level < 0.9)
        return 0.75;
    else if (level >= 0.9 && level < 1.0)
        return 0.9;
    else
        return 1.0;
}

QPointF topCenter(const QRectF& rect)
{
    return {rect.center().x(), rect.top()};
}

QPointF leftCenter(const QRectF& rect)
{
    return {rect.left(), rect.center().y()};
}

QPointF rightCenter(const QRectF& rect)
{
    return {rect.right(), rect.center().y()};
}

QPointF bottomCenter(const QRectF& rect)
{
    return {rect.center().x(), rect.bottom()};
}

QPixmap imageToPixmap(const QImage& image)
{
    QPixmap pixmap(QPixmap::fromImage(image));
    pixmap.setDevicePixelRatio(image.devicePixelRatio());
    return pixmap;
}

QStringList anchorLineNames()
{
    static const QStringList anchorLineNames {
        "anchors.top",
        "anchors.bottom",
        "anchors.left",
        "anchors.right",
        "anchors.horizontalCenter",
        "anchors.verticalCenter",
        "anchors.baseline",
        "anchors.fill",
        "anchors.centerIn"
    };
    return anchorLineNames;
}

QStringList anchorPropertyNames()
{
    static const QStringList anchorPropertyNames {
        "anchors.margins",
        "anchors.topMargin",
        "anchors.bottomMargin",
        "anchors.leftMargin",
        "anchors.rightMargin",
        "anchors.horizontalCenterOffset",
        "anchors.verticalCenterOffset",
        "anchors.baselineOffset",
        "anchors.alignWhenCentered"
    };
    return anchorPropertyNames;
}

void disableWheelEvent(QWidget* widget)
{
    class WheelDisabler final : public QObject {
        bool eventFilter(QObject* object, QEvent* event) override {
            if (event->type() == QEvent::Wheel) {
                event->ignore();
                return true;
            }
            return QObject::eventFilter(object, event);
        }
    };
    static WheelDisabler disabler;
    widget->installEventFilter(&disabler);
}

QFont defaultFont()
{
    QFont font;
    font.setFamily("Roboto");
    font.setPixelSize(13);
    return font;
}

QFont systemDefaultFont()
{
    QFont font;
#if defined(Q_OS_MACOS)
    if (QOperatingSystemVersion::current() >= QOperatingSystemVersion::MacOSCatalina)
        font.setFamily("SF UI Display");
    else
        font.setFamily(".SF NS Display");
#elif defined(Q_OS_WIN)
    font.setFamily("Segoe UI");
#endif
    font.setPixelSize(13);
    return font;
}

// Shortcut events are sent based on the shortcut context that's set on the action/shortcut.
// If shortcut context allows the shortcut to get triggered even if its window has the focus,
// then there might be an ambiguity among all the other widgets who has shortcuts in the same
// window shortcut context. In this case, the widget who has the focus gets the shortcut event.
// But if the widget that has focus doesn't assign a shortcut on it, it might not even be able
// to get key press events when if there are another widgets which have shortcuts for spesific
// keys even though they didn't even had the focus. So, in such cases, if you override the
// shortcut against one widget's good, then that widget will be getting the shortcut and key
// press events. Others will only get key press events. In this way, shortcut event propagation
// can be blocked against one widget's good.
void overrideShortcutFor(QWidget* widget, const std::function<bool(QKeyEvent*)>& condition)
{
    static QMap<QObject*, std::function<bool(QKeyEvent*)>> objects;
    class OverrideShortcut final : public QObject {
        bool eventFilter(QObject* watched, QEvent* event) override {
            if (objects.contains(watched) && event->type() == QEvent::ShortcutOverride) {
                auto keyEvent = static_cast<QKeyEvent*>(event);
                if (objects.value(watched)(keyEvent))
                    event->accept();
            }
            return QObject::eventFilter(watched, event);
        }
    };
    static OverrideShortcut shortcutOverrider;
    widget->installEventFilter(&shortcutOverrider);
    objects.insert(widget, condition);
}

void setFocusRing(QWidget* widget, bool showFocusRing)
{
    widget->setProperty(Internal::showFocusRingProperty, showFocusRing);
}

bool isFocusRingSet(QWidget* widget)
{
    return widget->property(Internal::showFocusRingProperty).toBool();
}

bool isDirAncestor(const QDir& ancestor, const QString& path)
{
    QFileInfo fileInfo(path);
    QDir dir = fileInfo.isDir() ? QDir(path) : fileInfo.dir();
    if (dir == ancestor)
        return true;
    while (dir.cdUp()) {
        if (dir == ancestor)
            return true;
    }
    return false;
}

QString shortcutSymbol(const QKeySequence& seq)
{
    return QStringLiteral(" <span style='font-size:10px;color:#656565;'>(%1)</span>")
            .arg(seq.toString(QKeySequence::NativeText));
}

QByteArray resourceData(const QString& path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly))
        return QByteArray();
    return file.readAll();
}

void updateToolTip(QWidget* widget, const QString& toolTip)
{
    widget->setToolTip(toolTip);
    if (QToolTip::isVisible()) {
        const QPoint& globalPos = QCursor::pos();
        const QPoint& pos = widget->mapFromGlobal(globalPos);
        if (pos.x() >= 0 && pos.y() >= 0 && pos.x() < widget->width() && pos.y() < widget->height())
            QToolTip::showText(globalPos, toolTip);
    }
}

void updateToolTip(QWidget* widget, const QString& toolTip, const QRect& region)
{
    if (QToolTip::isVisible()) {
        const QPoint& globalPos = QCursor::pos();
        const QPoint& pos = widget->mapFromGlobal(globalPos);
        if (pos.x() >= region.left() && pos.y() >= region.top()
                && pos.x() < region.right() && pos.y() < region.bottom()) {
            QToolTip::showText(globalPos, toolTip);
        }
    }
}
} // UtilityFunctions