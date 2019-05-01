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
    static QStringList projects();
    static QStringList projectNames();

    static QString uid();
    static QString dir(const QString& = instance()->uid());
    static QString name(const QString& = instance()->uid());
    static QString description(const QString& = instance()->uid());
    static QDateTime crDate(const QString& = instance()->uid());
    static QDateTime mfDate(const QString& = instance()->uid());
    static qint64 size(const QString& = instance()->uid());

    static void stop();
    static bool start(const QString& uid);
    static void updateSize(const QString& uid);
    static void updateLastModification(const QString& uid);
    static void changeName(const QString& uid, const QString& name);
    static void changeDescription(const QString& uid, const QString& desc);
    static bool importProject(const QString& filePath, QString* uid);
    static bool exportProject(const QString& uid, const QString& filePath);
    static bool newProject(int templateNumber, const QString& name, const QString& description,
                           const QDateTime& crDate);

signals:
    void started();
    void stopped();

private:
    explicit ProjectManager(QObject* parent = nullptr);
    ~ProjectManager();

private:
    static ProjectManager* s_instance;
    static QString s_currentUid;
};

#endif // PROJECTMANAGER_H
