#include <buildinfo.h>
#include <buildmodel.h>
#include <hashfactory.h>

#include <QFile>
#include <QStandardPaths>
#include <QCborMap>
#include <QCborArray>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

static QJsonObject handleMap(const QCborMap& map);

static QString packageSuffixFromRequest(const QCborMap& request)
{
    if (request.value(QLatin1String("platform")).toString() == QLatin1String("android")) {
        if (request.value(QLatin1String("aab")).toBool(false))
            return QLatin1String(".aab");
        else
            return QLatin1String(".apk");
    }
    return QString();
}

static QJsonArray handleArray(const QCborArray& array)
{
    QJsonArray jarray;
    for (const QCborValue& value : array) {
        if (value.isMap())
            jarray.append(handleMap(value.toMap()));
        else if (value.isArray())
            jarray.append(handleArray(value.toArray()));
        else if (value.isByteArray())
            jarray.append(QLatin1String("<raw_data>"));
        else
            jarray.append(value.toJsonValue());
    }
    return jarray;
}

static QJsonObject handleMap(const QCborMap& map)
{
    QJsonObject object;
    for (const QCborValue& key : map.keys()) {
        const QCborValue& value = map.value(key);
        if (value.isMap())
            object.insert(key.toString(), handleMap(value.toMap()));
        else if (value.isArray())
            object.insert(key.toString(), handleArray(value.toArray()));
        else if (value.isByteArray())
            object.insert(key.toString(), QLatin1String("<raw_data>"));
        else
            object.insert(key.toString(), value.toJsonValue());
    }
    return object;
}

QStringList BuildInfo::s_paths;

BuildInfo::BuildInfo(const QCborMap& request, QObject* parent) : QObject(parent)
  , m_identifier(HashFactory::generate())
  , m_request(request)
  , m_errorFlag(false)
  , m_speed(0)
  , m_totalBytes(0)
  , m_transferredBytes(0)
  , m_state(BuildModel::Uploading)
{
    const QString& suffix = packageSuffixFromRequest(request);
    const QString& baseName = request.value(QLatin1String("name")).toString();
    const QString& basePath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (!suffix.isEmpty() && !baseName.isEmpty() && !basePath.isEmpty()) {
        QString fullPath = basePath + QLatin1Char('/') + baseName + suffix;
        if (!QFile::exists(fullPath) && !s_paths.contains(fullPath.toLower())) {
            m_path = fullPath;
            s_paths.append(m_path.toLower());
        } else {
            int i = 1;
            forever {
                fullPath = basePath + QLatin1Char('/') + baseName + QString::number(i++) + suffix;
                if (!QFile::exists(fullPath) && !s_paths.contains(fullPath.toLower())) {
                    m_path = fullPath;
                    s_paths.append(m_path.toLower());
                    break;
                }
            }
        }
    }
    addStatus(tr("Request Details:"));
    addStatus(QJsonDocument(handleMap(request)).toJson());
}

BuildInfo::~BuildInfo()
{
    s_paths.removeOne(m_path.toLower());
}

QByteArray BuildInfo::identifier() const
{
    return m_identifier;
}

const QCborMap& BuildInfo::request() const
{
    return m_request;
}

const QString& BuildInfo::path() const
{
    return m_path;
}

QList<BuildInfo::Block>& BuildInfo::recentBlocks()
{
    return m_recentBlocks;
}

QByteArray BuildInfo::uid() const
{
    return m_uid;
}

void BuildInfo::setUid(const QByteArray& uid)
{
    m_uid = uid;
}

bool BuildInfo::hasError() const
{
    return m_errorFlag;
}

void BuildInfo::setErrorFlag(bool errorFlag)
{
    m_errorFlag = errorFlag;
}

QString BuildInfo::statusTip() const
{
    return m_statusTip;
}

QString BuildInfo::status() const
{
    return m_status;
}

void BuildInfo::addStatus(const QString& status)
{
    const QString& timestamp = QTime::currentTime().toString("hh:mm:ss");
    QTextStream stream(status.toUtf8());
    QString line;
    while (stream.readLineInto(&line)) {
        if (line.contains(QRegularExpression("[^\\r\\n\\t\\f\\v ]")))
            m_status.append(timestamp + QLatin1Char(' ') + line + QLatin1Char('\n'));
        else
            m_status.append(line);
    }
    line = m_status.trimmed();
    int begin = line.lastIndexOf(QLatin1Char('\n')) + 1;
    int end = line.lastIndexOf(QRegularExpression("[^\\r\\n\\t\\f\\v ]")) + 1;
    if (begin > 0 && end > begin)
        line = line.mid(begin, end - begin);
    m_statusTip = line.mid(9);
}

QByteArray BuildInfo::payloadUid() const
{
    return m_payloadUid;
}

void BuildInfo::setPayloadUid(const QByteArray& payloadUid)
{
    m_payloadUid = payloadUid;
}

QTime BuildInfo::timeLeft() const
{
    return m_timeLeft;
}

void BuildInfo::setTimeLeft(const QTime& timeLeft)
{
    m_timeLeft = timeLeft;
}

qreal BuildInfo::speed() const
{
    return m_speed;
}

void BuildInfo::setSpeed(qreal speed)
{
    m_speed = speed;
}

int BuildInfo::totalBytes() const
{
    return m_totalBytes;
}

void BuildInfo::setTotalBytes(int totalBytes)
{
    m_totalBytes = totalBytes;
}

int BuildInfo::transferredBytes() const
{
    return m_transferredBytes;
}

void BuildInfo::setTransferredBytes(int transferredBytes)
{
    m_transferredBytes = transferredBytes;
}

int BuildInfo::state() const
{
    return m_state;
}

void BuildInfo::setState(int state)
{
    m_state = state;
}
