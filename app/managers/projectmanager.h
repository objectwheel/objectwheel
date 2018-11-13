#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QObject>

class ProjectManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ProjectManager)

    friend class ApplicationCore;

public:
    static ProjectManager* instance();

    static QString currentDbTime();
    static QString currentUiTime();
    static QString toDbTime(const QString& uiTime);
    static QString toUiTime(const QString& dbTime);
    static QDateTime fromDb(const QString& dbTime);
    static QDateTime fromUi(const QString& uiTime);

    static QStringList projects();
    static QStringList projectNames();

    static QString hash();
    static QString dir(const QString& = instance()->hash());
    static QString name(const QString& = instance()->hash());
    static QString description(const QString& = instance()->hash());
    static QString owner(const QString& = instance()->hash());
    static QString crDate(const QString& = instance()->hash());
    static QString mfDate(const QString& = instance()->hash());
    static QString size(const QString& = instance()->hash());

    static void stop();
    static bool start(const QString& hash);
    static void updateSize(const QString& hash);
    static void updateLastModification(const QString& hash);
    static void changeName(const QString& hash, const QString& name);
    static void changeDescription(const QString& hash, const QString& desc);
    static bool importProject(const QString& filePath);
    static bool exportProject(const QString& hash, const QString& filePath);
    static bool newProject(int templateNumber, const QString& name, const QString& description,
                           const QString& owner, const QString& crDate);

signals:
    void started();
    void stopped();

private:
    explicit ProjectManager(QObject* parent = nullptr);
    ~ProjectManager();

private:
    static ProjectManager* s_instance;
    static QString s_currentHash;
};

#endif // PROJECTMANAGER_H
