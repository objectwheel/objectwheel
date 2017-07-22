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

    private slots:
        void setVariantProperty(const QString& fileName, const QString& property, const QVariant& value);
        void removeVariantProperty(const QString& fileName, const QString& property);

    private:
        QmlJS::ModelManagerInterface* _modelManager;
};

#endif // PARSERWORKER_H
