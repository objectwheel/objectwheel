#ifndef UTILITYFUNCTIONS_H
#define UTILITYFUNCTIONS_H

#include <Qt>
#include <QFont>
#include <QMessageBox>
#include <QCborArray>
#include <renderinfo.h>

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

void pushCborHelper(QCborArray&);
void pullCborHelper(QCborArray&);
void pushHelper(QDataStream&);
void pullHelper(QDataStream&);

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

void trimCommentsAndStrings(QTextDocument* document);
void copyFiles(const QString& rootPath, const QList<QUrl>& urls, QWidget* parent);
void expandUpToRoot(QTreeView* view, const QModelIndex& index, const QModelIndex& rootIndex);
void centralizeWidget(QWidget* widget);
void adjustFontWeight(QWidget* widget, QFont::Weight weight);
void adjustFontPixelSize(QWidget* widget, int advance);
bool hasHover(const QWidget* widget);
bool isEmailFormatCorrect(const QString& email);
bool isPasswordFormatCorrect(const QString& password);
bool isPasswordHashFormatCorrect(const QString& hash); // SHA3-512
QWidget* createSpacingWidget(const QSize& size);
QWidget* createSpacerWidget(Qt::Orientation orientation);
QWidget* createSeparatorWidget(Qt::Orientation orientation);
QWindow* window(const QWidget* widget);
QRectF verticalAlignedRect(const QSizeF& size, const QRectF& rect, qreal left = 0);
QRectF horizontalAlignedRect(const QSizeF& size, const QRectF& rect, qreal top = 0);
QPoint centerPos(const QSize& size);
QString increasedNumberedText(const QString& text, bool addSpace, bool trim);
QString toPrettyBytesString(qint64 bytes);
bool comboContainsWord(QComboBox* comboBox, const QString& word);
QRectF itemGeometry(const QVector<PropertyNode>& properties);
QVariant itemProperty(const QString& propertyName, const QVector<PropertyNode>& properties);
QVariantMap localDeviceInfo();
QString stringify(const QString& text);
QIcon deviceIcon(const QVariantMap& deviceInfo);
QString deviceUid(const QVariantMap& deviceInfo);
QString deviceName(const QVariantMap& deviceInfo);
QString deviceInfoToolTip(const QVariantMap& deviceInfo);
QString deviceUid(const QAction* action);
QString buildCpuArchitecture();
void setDeviceInfo(QAction* action, const QVariantMap& deviceInfo);
QMessageBox::StandardButton showMessage(QWidget* parent, const QString& title, const QString& text,
                                        QMessageBox::Icon icon = QMessageBox::Warning,
                                        QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                                        QMessageBox::StandardButton defaultButton = QMessageBox::Ok,
                                        bool modal = true);
QByteArray generateJunk(int sizeInBytes);
QByteArray generateAutoLoginHash(const QString& email, const QString& pwhash);
QByteArray generatePasswordHash(const QByteArray& password);
bool testAutoLogin(const QByteArray& hash, QString* email = nullptr, QString* pwhash = nullptr);
bool testPassword(const QByteArray& password, const QByteArray& hash);
void cleanSensitiveInformation(QString& message);
QStringList zoomTexts();
QVector<qreal> zoomLevels();
QString zoomLevelToText(qreal level);
qreal textToZoomLevel(const QString& text);
qreal roundZoomLevel(qreal level);
QPointF topCenter(const QRectF& rect);
QPointF leftCenter(const QRectF& rect);
QPointF rightCenter(const QRectF& rect);
QPointF bottomCenter(const QRectF& rect);
QPixmap imageToPixmap(const QImage& image);
QStringList anchorLineNames();
QStringList anchorPropertyNames();
void disableWheelEvent(QWidget* widget);
QFont defaultFont();
QFont systemDefaultFont();
QFont systemTitleFont();
void overrideShortcutFor(QWidget* widget, const std::function<bool(QKeyEvent*)>& condition);
void setFocusRing(QWidget* widget, bool showFocusRing);
bool isFocusRingSet(QWidget* widget);
bool isDirAncestor(const QDir& ancestor, const QString& path);
QString shortcutSymbol(const QKeySequence& seq);
QByteArray resourceData(const QString& path);
void updateToolTip(QWidget* widget, const QString& toolTip);
void updateToolTip(QWidget* widget, const QString& toolTip, const QRect& region);

} // UtilityFunctions

#endif // UTILITYFUNCTIONS_H