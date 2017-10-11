#ifndef PARSERWORKER_H
#define PARSERWORKER_H

#include <QObject>

namespace QmlJS {
    class ModelManagerInterface;
}

namespace QmlDesigner {
    class Model;
    class RewriterView;
    class NotIndentingTextEditModifier;
}

class ParserWorker : public QObject
{
        Q_OBJECT

    public:
        explicit ParserWorker(QObject *parent = 0);
        QString typeName(const QByteArray& data) const;

    public slots:
        void setVariantProperty(QByteArray& data, const QString& fileName, const QString& property, const QVariant& value);
        void setVariantProperty(const QString& fileName, const QString& property, const QVariant& value);
        void removeVariantProperty(const QString& fileName, const QString& property);

    signals:
        void done();

    private:
        static QmlJS::ModelManagerInterface* _modelManager;
};

#endif // PARSERWORKER_H
