#ifndef TOOLSBACKEND_H
#define TOOLSBACKEND_H

#include <QObject>
#include <QUrl>

class QJsonObject;
class ToolboxTree;

class ToolsBackend : QObject
{
        Q_OBJECT
        Q_DISABLE_COPY(ToolsBackend)

    public:
        struct ChangeSet {
            QString name;
            QString toolPath;
            QString category;
            QString iconPath;
        };

    public:
        static ToolsBackend* instance();
        bool addTool(
            const QString& toolPath,
            const bool select,
            const bool qrc = false
        );
        void removeTool(const QString& toolPath);
        void changeTool(const ChangeSet& changeSet);
        void addToolboxTree(ToolboxTree* toolboxTree);
        QString toolsDir() const;
        QStringList categories() const;

    private:
        void fillTree(ToolboxTree* tree);
        bool addToTree(const QString& toolPath, ToolboxTree* tree);

    public slots:
        void newTool();
        void resetTools();
        void downloadTools(const QUrl& url = QUrl());

    private:
        ToolsBackend() {}
        QList<ToolboxTree*> _toolboxTreeList;
};

#endif // TOOLSBACKEND_H
