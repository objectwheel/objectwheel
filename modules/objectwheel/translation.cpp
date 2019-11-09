#include <translation.h>
#include <QQmlEngine>
#include <QTranslator>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDir>

Translation::Translation(QQmlEngine* engine, QObject* parent) : QObject(parent)
  , m_engine(engine)
{
}

void Translation::clear()
{
    load(QString());
}

void Translation::load(const QString& translationFilePath)
{
    static QTranslator* translator = nullptr;

    if (translator) {
        QCoreApplication::removeTranslator(translator);
        translator->deleteLater();
        translator = nullptr;
        m_engine->retranslate();
    }

    QString filePath = translationFilePath;
    filePath.replace("file://", "");

    if (!translationFilePath.isEmpty() && QFileInfo::exists(filePath)) {
        translator = new QTranslator(this);
        if (translator->load(filePath)) {
            QCoreApplication::installTranslator(translator);
            m_engine->retranslate();
        }
    }
}
