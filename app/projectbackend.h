#ifndef PROJECTBACKEND_H
#define PROJECTBACKEND_H

#include <QObject>

class ProjectBackend final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ProjectBackend)

    friend class BackendManager;

public:
    static ProjectBackend* instance();

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
    static void updateSize();
    static void updateLastModification();
    static void changeName(const QString& hash, const QString& name);
    static void changeDescription(const QString& hash, const QString& desc);
    static bool importProject(const QString& filePath);
    static bool exportProject(const QString& hash, const QString& filePath);
    static bool newProject(int templateNumber, const QString& name, const QString& description,
                           const QString& owner, const QString& crDate,const QString& size);
signals:
    void started();
    void stopped();

private:
    explicit ProjectBackend(QObject* parent = nullptr);
    ~ProjectBackend();

private:
    static ProjectBackend* s_instance;
    static QString s_currentHash;
};

#endif // PROJECTBACKEND_H
