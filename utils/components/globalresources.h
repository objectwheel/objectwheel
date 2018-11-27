#ifndef GLOBALRESOURCES_H
#define GLOBALRESOURCES_H

#include <QObject>
#include <QUrl>

class GlobalResources : public QObject
{
    Q_OBJECT

    friend class ApplicationCore;

public:
    static GlobalResources* instance();

public slots:
    QUrl url() const;
    QString path() const;

private:
    explicit GlobalResources(const std::function<QString()>& projectDirectory, QObject* parent = nullptr);
    ~GlobalResources();

private:
    static GlobalResources* s_instance;
    const std::function<QString()> m_projectDirectoryFunction;
};

#endif // GLOBALRESOURCES_H