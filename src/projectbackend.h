#ifndef PROJECTBACKEND_H
#define PROJECTBACKEND_H

#include <QObject>

class ProjectBackend : public QObject
{
		Q_OBJECT
        Q_DISABLE_COPY(ProjectBackend)

	public:
        static ProjectBackend* instance();

        bool importProject(const QString& filePath);
        bool exportProject(const QString& hash, const QString& filePath);

        bool newProject(
            const QString& name,
            const QString& description,
            const QString& owner,
            const QString& crDate,
            const QString& size
        );

        void changeName(const QString& hash, const QString& name);
        void changeDescription(const QString& hash, const QString& desc);

        const QString& hash() const;
        QStringList projects() const;
        QString dir(const QString& = instance()->hash()) const;
        QString name(const QString& = instance()->hash()) const;
        QString description(const QString& = instance()->hash()) const;
        QString owner(const QString& = instance()->hash()) const;
        QString crDate(const QString& = instance()->hash()) const;
        QString mfDate(const QString& = instance()->hash()) const;
        QString size(const QString& = instance()->hash()) const;

    public slots:
        void stop();
        bool start(const QString& hash);

    private slots:
        void updateSize();
        void updateLastModification();

    signals:
        void started();
        void stopped();

	private:
        ProjectBackend() {}

    private:
        QString _currentHash;
};

#endif // PROJECTBACKEND_H
