#ifndef UTILITYFUNCTIONS_H
#define UTILITYFUNCTIONS_H

#include <utils_global.h>
#include <renderinfo.h>

#include <QFont>
#include <QMessageBox>
#include <QCborArray>
#include <QStyle>

#if defined(Q_CC_CLANG)
#  define QT_WARNING_DISABLE QT_WARNING_DISABLE_CLANG
#elif defined(Q_CC_GNU)
#  define QT_WARNING_DISABLE QT_WARNING_DISABLE_GCC
#else
#  define QT_WARNING_DISABLE
#endif

class QTextDocument;
class QWidget;
class QUrl;
class QModelIndex;
class QTreeView;
class QWindow;
class QQmlEngine;
class QAbstractButton;
class QAbstractItemView;
class QComboBox;
class QMarginsF;
class QAction;
class QDir;

namespace UtilityFunctions {

namespace Internal {

UTILS_EXPORT void pushCborHelper(QCborArray&);
UTILS_EXPORT void pullCborHelper(QCborArray&);
UTILS_EXPORT void pushHelper(QDataStream&);
UTILS_EXPORT void pullHelper(QDataStream&);

template <typename Arg, typename... Args>
void pushCborHelper(QCborArray& array, Arg&& arg, Args&&... args)
{
    array.append(std::forward<Arg>(arg));
    pushCborHelper(array, std::forward<Args>(args)...);
}

template <typename Arg, typename... Args>
void pullCborHelper(QCborArray& array, Arg&& arg, Args&&... args)
{
    if (!array.isEmpty())
        std::forward<Arg>(arg) = array.takeFirst().toVariant().value<std::remove_reference_t<Arg>>();
    pullCborHelper(array, std::forward<Args>(args)...);
}

template <typename Arg, typename... Args>
void pushHelper(QDataStream& stream, Arg&& arg, Args&&... args)
{
    stream << std::forward<Arg>(arg);
    pushHelper(stream, std::forward<Args>(args)...);
}

template <typename Arg, typename... Args>
void pullHelper(QDataStream& stream, Arg&& arg, Args&&... args)
{
    stream >> std::forward<Arg>(arg);
    pullHelper(stream, std::forward<Args>(args)...);
}

} // Internal

template <typename... Args>
QByteArray pushCbor(Args&&... args)
{
    QCborArray array;
    Internal::pushCborHelper(array, std::forward<Args>(args)...);
    return array.toCborValue().toCbor();
}

template <typename... Args>
void pullCbor(const QByteArray& data, Args&&... args)
{
    QCborArray array(QCborValue::fromCbor(data).toArray());
    Internal::pullCborHelper(array, std::forward<Args>(args)...);
}

template <typename... Args>
QByteArray push(Args&&... args)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_12);
    Internal::pushHelper(stream, std::forward<Args>(args)...);
    return data;
}

template <typename... Args>
void pull(const QByteArray& data, Args&&... args)
{
    QDataStream stream(data);
    stream.setVersion(QDataStream::Qt_5_12);
    Internal::pullHelper(stream, std::forward<Args>(args)...);
}

UTILS_EXPORT void trimCommentsAndStrings(QTextDocument* document);
UTILS_EXPORT void copyFiles(const QString& rootPath, const QList<QUrl>& urls, QWidget* parent);
UTILS_EXPORT void expandUpToRoot(QTreeView* view, const QModelIndex& index, const QModelIndex& rootIndex);
UTILS_EXPORT void centralizeWidget(QWidget* widget);
UTILS_EXPORT void adjustFontWeight(QWidget* widget, QFont::Weight weight);
UTILS_EXPORT void adjustFontPixelSize(QWidget* widget, int advance);
UTILS_EXPORT bool hasHover(const QWidget* widget);
UTILS_EXPORT bool isEmailFormatCorrect(const QString& email);
UTILS_EXPORT bool isPasswordFormatCorrect(const QString& password);
UTILS_EXPORT bool isPasswordHashFormatCorrect(const QString& hash); // SHA3-512
UTILS_EXPORT QString hashPassword(const QString& password); // SHA3-512
UTILS_EXPORT QWidget* createSpacingWidget(const QSize& size);
UTILS_EXPORT QWidget* createSpacerWidget(Qt::Orientation orientation);
UTILS_EXPORT QWidget* createSeparatorWidget(Qt::Orientation orientation);
UTILS_EXPORT QWindow* window(const QWidget* widget);
UTILS_EXPORT QRectF verticalAlignedRect(const QSizeF& size, const QRectF& rect, qreal left = 0);
UTILS_EXPORT QRectF horizontalAlignedRect(const QSizeF& size, const QRectF& rect, qreal top = 0);
UTILS_EXPORT QPoint centerPos(const QSize& size);
UTILS_EXPORT QString increasedNumberedText(const QString& text, bool addSpace, bool trim);
UTILS_EXPORT QString toPrettyBytesString(qint64 bytes);
UTILS_EXPORT bool comboContainsWord(QComboBox* comboBox, const QString& word);
UTILS_EXPORT QRectF itemGeometry(const QVector<PropertyNode>& properties);
UTILS_EXPORT QVariant itemProperty(const QString& propertyName, const QVector<PropertyNode>& properties);
UTILS_EXPORT QVariantMap localDeviceInfo();
UTILS_EXPORT QString stringify(const QString& text);
UTILS_EXPORT QIcon deviceIcon(const QVariantMap& deviceInfo);
UTILS_EXPORT QString deviceUid(const QVariantMap& deviceInfo);
UTILS_EXPORT QString deviceName(const QVariantMap& deviceInfo);
UTILS_EXPORT QString deviceInfoToolTip(const QVariantMap& deviceInfo);
UTILS_EXPORT QString deviceUid(const QAction* action);
UTILS_EXPORT void setDeviceInfo(QAction* action, const QVariantMap& deviceInfo);
UTILS_EXPORT QMessageBox::StandardButton showMessage(QWidget* parent, const QString& title, const QString& text,
                                                     QMessageBox::Icon icon = QMessageBox::Warning,
                                                     QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                                                     QMessageBox::StandardButton defaultButton = QMessageBox::Ok,
                                                     bool modal = true);
UTILS_EXPORT QByteArray generateJunk(int sizeInBytes);
UTILS_EXPORT QByteArray generateAutoLoginHash(const QString& email, const QString& pwhash);
UTILS_EXPORT QByteArray generatePasswordHash(const QByteArray& password);
UTILS_EXPORT bool testAutoLogin(const QByteArray& hash, QString* email = nullptr, QString* pwhash = nullptr);
UTILS_EXPORT bool testPassword(const QByteArray& password, const QByteArray& hash);
UTILS_EXPORT void cleanSensitiveInformation(QString& message);
UTILS_EXPORT QStringList zoomTexts();
UTILS_EXPORT QVector<qreal> zoomLevels();
UTILS_EXPORT QString zoomLevelToText(qreal level);
UTILS_EXPORT qreal textToZoomLevel(const QString& text);
UTILS_EXPORT qreal roundZoomLevel(qreal level);
UTILS_EXPORT QPointF topCenter(const QRectF& rect);
UTILS_EXPORT QPointF leftCenter(const QRectF& rect);
UTILS_EXPORT QPointF rightCenter(const QRectF& rect);
UTILS_EXPORT QPointF bottomCenter(const QRectF& rect);
UTILS_EXPORT QPixmap imageToPixmap(const QImage& image);
UTILS_EXPORT QStringList anchorLineNames();
UTILS_EXPORT QStringList anchorPropertyNames();
UTILS_EXPORT QStringList countryList();
UTILS_EXPORT void disableWheelEvent(QWidget* widget);
UTILS_EXPORT QFont defaultFont();
UTILS_EXPORT QFont systemDefaultFont();
UTILS_EXPORT QFont systemTitleFont();
UTILS_EXPORT QFont thickerFont(const QFont& font);
UTILS_EXPORT void overrideShortcutFor(QWidget* widget, const std::function<bool(QKeyEvent*)>& condition);
UTILS_EXPORT void setFocusRing(QWidget* widget, bool showFocusRing);
UTILS_EXPORT bool isFocusRingSet(QWidget* widget);
UTILS_EXPORT bool isDirAncestor(const QDir& ancestor, const QString& path);
UTILS_EXPORT QString shortcutSymbol(const QKeySequence& seq);
UTILS_EXPORT QByteArray resourceData(const QString& path);
UTILS_EXPORT void updateToolTip(QWidget* widget, const QString& toolTip);
UTILS_EXPORT void updateToolTip(QWidget* widget, const QString& toolTip, const QRect& region);
UTILS_EXPORT QMargins itemViewMargins(const QAbstractItemView* view);
UTILS_EXPORT QMargins layoutItemMargins(const QWidget* widget);
UTILS_EXPORT void setItemViewMargins(QAbstractItemView* view, const QMargins& margins);
UTILS_EXPORT void setLayoutItemMargins(QWidget* widget, const QMargins& margins);

} // UtilityFunctions

#endif // UTILITYFUNCTIONS_H