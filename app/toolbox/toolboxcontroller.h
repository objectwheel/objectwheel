#ifndef TOOLBOXCONTROLLER_H
#define TOOLBOXCONTROLLER_H

#include <QObject>

class ToolboxPane;
class ToolboxItem;

class ToolboxController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ToolboxController)

public:
    explicit ToolboxController(ToolboxPane* m_toolboxPane, QObject* parent = nullptr);

public slots:
    void discharge();

private slots:
    void onToolboxPress(ToolboxItem* item);
    void onSearchTextEdit(const QString& text);

private:
    void fillPane();

signals:
    void filled();

private:
    ToolboxPane* m_toolboxPane;
};

#endif // TOOLBOXCONTROLLER_H
