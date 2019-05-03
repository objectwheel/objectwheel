#ifndef UTILITYFUNCTIONS_H
#define UTILITYFUNCTIONS_H

#include <Qt>
#include <QFont>
#include <QMessageBox>

#include <serializeenum.h>
#include <previewresult.h>

class QTextDocument;
class QWidget;
class QUrl;
class QModelIndex;
class QTreeView;
class QWindow;
class QQmlEngine;
class QAbstractButton;
class QAbstractItemView;
class QQmlError;
class QComboBox;
class QMarginsF;
class QAction;

namespace UtilityFunctions {

namespace Internal {

void pushHelper(QDataStream&);
void pullHelper(QDataStream&);

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
QWidget* createSpacingWidget(const QSize& size);
QWidget* createSpacerWidget(Qt::Orientation orientation);
QWidget* createSeparatorWidget(Qt::Orientation orientation);
QWindow* window(const QWidget* widget);
QRectF verticalAlignedRect(const QSizeF& size, const QRectF& rect, qreal left = 0);
QRectF horizontalAlignedRect(const QSizeF& size, const QRectF& rect, qreal top = 0);
QPixmap pixmap(QAbstractButton* button, const QIcon& icon, const QSizeF& size);
QPixmap scaled(const QPixmap& pixmap, const QSize& size);
QIcon iconForQmlError(const QQmlError& error, const QAbstractItemView* view);
QPoint centerPos(const QSize& size);
QString increasedNumberedText(const QString& text, bool addSpace, bool trim);
QString toPrettyBytesString(qint64 bytes);
bool comboContainsWord(QComboBox* comboBox, const QString& word);
QRectF getGeometryFromProperties(const QList<PropertyNode>& properties);
QMarginsF getMarginsFromProperties(const QList<PropertyNode>& properties);
void putMarginsToProperties(QMap<QString, QVariant>& properties, const QMarginsF& margins);
QVariant getProperty(const QString& property, const QList<PropertyNode>& properties);
Enum getEnum(const QString& name, const QList<PropertyNode>& properties);
QVariantMap localDeviceInfo();
QIcon deviceIcon(const QVariantMap& deviceInfo);
QString deviceUid(const QVariantMap& deviceInfo);
QString deviceName(const QVariantMap& deviceInfo);
QString deviceInfoToolTip(const QVariantMap& deviceInfo);
QString deviceUid(const QAction* action);
void setDeviceInfo(QAction* action, const QVariantMap& deviceInfo);
QMessageBox::StandardButton showMessage(QWidget* parent, const QString& title, const QString& text,
                                        QMessageBox::Icon icon = QMessageBox::Warning,
                                        QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                                        QMessageBox::StandardButton defaultButton = QMessageBox::Ok,
                                        bool modal = true);
} // UtilityFunctions

#endif // UTILITYFUNCTIONS_H