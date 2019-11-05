#ifndef TOOLBOXCONTROLLER_H
#define TOOLBOXCONTROLLER_H

#include <QObject>

class ToolboxPane;
class ToolboxItem;
class QDrag;

class ToolboxController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ToolboxController)

public:
    explicit ToolboxController(ToolboxPane* m_toolboxPane, QObject* parent = nullptr);

public slots:
    void discharge();

private slots:
    void onToolboxSettingsChange();
    void onDocumentManagerInitialization();
    void onSearchEditEdit(const QString& searchTerm);
    void onToolboxItemPress(ToolboxItem* item);

private:
    QDrag* establishDrag(ToolboxItem* item);

private:
    ToolboxPane* m_toolboxPane;
};

#endif // TOOLBOXCONTROLLER_H
