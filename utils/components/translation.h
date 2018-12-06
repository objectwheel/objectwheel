#ifndef TRANSLATION_H
#define TRANSLATION_H

#include <QObject>

class QQmlEngine;

class Translation : public QObject
{
    Q_OBJECT

public:
    Translation(QQmlEngine* engine);

public slots:
    void clear();
    void load(const QString& translationFilePath);

private:
    QQmlEngine* m_engine;
};

#endif // TRANSLATION_H