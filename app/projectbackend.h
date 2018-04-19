#ifndef PROJECTBACKEND_H
#define PROJECTBACKEND_H

#include <QObject>

class ProjectBackend : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ProjectBackend)

public:
    static ProjectBackend* instance();

    bool newProject(
        int templateNumber,
        const QString& name,
        const QString& description,
        const QString& owner,
        const QString& crDate,
        const QString& size
    ) const;

    QStringList projectNames() const;
    QStringList projects() const;
    const QString& hash() const;
    QString dir(const QString& = instance()->hash()) const;
    QString name(const QString& = instance()->hash()) const;
    QString description(const QString& = instance()->hash()) const;
    QString owner(const QString& = instance()->hash()) const;
    QString crDate(const QString& = instance()->hash()) const;
    QString mfDate(const QString& = instance()->hash()) const;
    QString size(const QString& = instance()->hash()) const;

    void changeName(const QString& hash, const QString& name) const;
    void changeDescription(const QString& hash, const QString& desc) const;
    bool importProject(const QString& filePath) const;
    bool exportProject(const QString& hash, const QString& filePath) const;

public slots:
    void stop();
    bool start(const QString& hash);
    void updateSize() const;
    void updateLastModification() const;

signals:
    void started();
    void stopped();

private:
    ProjectBackend() {}

private:
    QString _currentHash;
};

#endif // PROJECTBACKEND_H
