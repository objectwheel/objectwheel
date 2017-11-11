#ifndef TOOLSMANAGER_H
#define TOOLSMANAGER_H

#include <QObject>
#include <QUrl>

class QJsonObject;
class ToolboxTree;

class ToolsManager : QObject
{
        Q_OBJECT
        Q_DISABLE_COPY(ToolsManager)

    public:
        static ToolsManager* instance();
        QString toolsDir() const;
        QStringList categories() const;

        bool addTool(const QString& toolPath);
        void removeTool(const QString& toolPath);
        void addToolboxTree(ToolboxTree* toolboxTree);
        void downloadTools(const QUrl& url = QUrl());

    public slots:
        void resetTools();
        void createNewTool();

    private:
        ToolsManager() {}
        QList<ToolboxTree*> _toolboxTreeList;
};

#endif // TOOLSMANAGER_H
