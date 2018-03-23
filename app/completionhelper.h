#ifndef COMPLETIONHELPER_H
#define COMPLETIONHELPER_H

#include <QObject>
#include <QCompleter>
#include <QStandardItemModel>

class QmlCodeEditor;

struct ExtractionResult {
        QStringList keywords;
        QStringList properties;
        QStringList functions;
        QStringList types;
};

class CompletionHelper : public QObject
{
        Q_OBJECT

    public:
        explicit CompletionHelper(QObject* parent = nullptr);

    public slots:
        void extractCompletions(const QString& text);

    signals:
        void extractionReady(const ExtractionResult& result);
};

#endif // COMPLETIONHELPER_H
