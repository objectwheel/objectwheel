#ifndef SAVEUTILS_H
#define SAVEUTILS_H

#include <QVariant>
#include <QDir>
#include <QBasicTimer>

class SaveUtils final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SaveUtils)

public:
    enum ControlProperties : quint32 {
        ControlId = 0x1000,
        ControlUid,
        ControlIcon,
        ControlToolName,
        ControlToolCategory,
        ControlVersion,
        ControlSignature
    };

    enum ProjectProperties : quint32 {
        ProjectName = 0x2000,
        ProjectDescription,
        ProjectCreationDate,
        ProjectModificationDate,
        ProjectSize,
        ProjectUid,
        ProjectTheme,
        ProjectHdpiScaling,
        ProjectVersion,
        ProjectSignature
    };

    enum UserProperties : quint32 {
        UserEmail = 0x3000,
        UserPassword,
        UserFirst,
        UserLast,
        UserCountry,
        UserCompany,
        UserTitle,
        UserPhone,
        UserIcon,
        UserPlan,
        UserRegistrationDate,
        UserLastOnlineDate,
        UserVersion,
        UserSignature
    };

    using ControlMap = QMap<ControlProperties, QVariant>;
    using ProjectMap = QMap<ProjectProperties, QVariant>;
    using UserMap = QMap<UserProperties, QVariant>;

public:
    static bool isForm(const QString& controlDir);
    static bool isControlValid(const QString& controlDir);
    static bool isProjectValid(const QString& projectDir);
    static bool isUserValid(const QString& userDir);

    static QString controlMetaFileName();
    static QString projectMetaFileName();
    static QString userMetaFileName();
    static QString mainQmlFileName();

    static QString toControlMetaFile(const QString& controlDir);
    static QString toProjectMetaFile(const QString& projectDir);
    static QString toUserMetaFile(const QString& userDir);
    static QString toMainQmlFile(const QString& controlDir);
    static QString toThisDir(const QString& controlDir);
    static QString toChildrenDir(const QString& controlDir);
    static QString toParentDir(const QString& controlDir);
    static QString toDesignsDir(const QString& projectDir);
    static QString toImportsDir(const QString& projectDir);
    static QString toOwDir(const QString& projectDir);
    static QString toGlobalDir(const QString& projectDir);

    static QString id(const QString& controlDir);
    static QString uid(const QString& controlDir);
    static QString name(const QString& controlDir);
    static QString category(const QString& controlDir);
    static QByteArray icon(const QString& controlDir);

    static bool projectHdpiScaling(const QString& projectDir);
    static qint64 projectSize(const QString& projectDir);
    static QString projectUid(const QString& projectDir);
    static QString projectName(const QString& projectDir);
    static QString projectDescription(const QString& projectDir);
    static QDateTime projectCreationDate(const QString& projectDir);
    static QDateTime projectModificationDate(const QString& projectDir);
    static QJsonValue projectTheme(const QString& projectDir);

    static quint32 userPlan(const QString& userDir);
    static QString userEmail(const QString& userDir);
    static QString userFirst(const QString& userDir);
    static QString userLast(const QString& userDir);
    static QString userCountry(const QString& userDir);
    static QString userCompany(const QString& userDir);
    static QString userTitle(const QString& userDir);
    static QString userPhone(const QString& userDir);
    static QByteArray userPassword(const QString& userDir);
    static QByteArray userIcon(const QString& userDir);
    static QDateTime userLastOnlineDate(const QString& userDir);
    static QDateTime userRegistrationDate(const QString& userDir);

    static ControlMap controlMap(const QString& controlDir);
    static ProjectMap projectMap(const QString& projectDir);
    static UserMap userMap(const QString& userDir);

    static QVariant property(const QString& controlDir, ControlProperties property);
    static QVariant property(const QString& projectDir, ProjectProperties property);
    static QVariant property(const QString& userDir, UserProperties property);

    static void setProperty(const QString& controlDir, ControlProperties property, const QVariant& value);
    static void setProperty(const QString& projectDir, ProjectProperties property, const QVariant& value);
    static void setProperty(const QString& userDir, UserProperties property, const QVariant& value);

    static void makeControlMetaFile(const QString& controlDir);
    static void makeProjectMetaFile(const QString& projectDir);
    static void makeUserMetaFile(const QString& userDir);

    static void sync();
    static void regenerateUids(const QString& topPath);
    static QStringList formPaths(const QString& projectDir);
    static QStringList childrenPaths(const QString& controlDir);

private:
    explicit SaveUtils(QObject* parent = nullptr);
    ~SaveUtils() override;

private:
    void timerEvent(QTimerEvent* event) override;

private:
    static QBasicTimer s_syncTimer;
    static QMap<QDir, ControlMap> s_controlCache;
    static QMap<QDir, ProjectMap> s_projectCache;
    static QMap<QDir, UserMap> s_userCache;
};

#endif // SAVEUTILS_H
