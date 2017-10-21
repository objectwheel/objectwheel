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
        QVariant variantProperty(const QByteArray& data, const QString& name) const;

    public slots:
        void setVariantProperty(QByteArray& data, const QString& fileName, const QString& property, const QVariant& value) const;
        void setVariantProperty(const QString& fileName, const QString& property, const QVariant& value) const;
        void removeVariantProperty(const QString& fileName, const QString& property) const;

    signals:
        void done() const;

    private:
        static QmlJS::ModelManagerInterface* _modelManager;
};

#endif // PARSERWORKER_H
