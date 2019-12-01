#ifndef ASSETSCONTROLLER_H
#define ASSETSCONTROLLER_H

#include <QObject>

class AssetsPane;
class QCompleter;
class FileSystemCompleterModel;

class AssetsController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AssetsController)

public:
    explicit AssetsController(AssetsPane* assetsPane, QObject* parent = nullptr);

public slots:
    void charge();
    void discharge();

private slots:
    void onModeComboBoxActivation();
    void onSearchEditReturnPress();
    void onCurrentDirChange(const QString& currentDir);

private:
    AssetsPane* m_assetsPane;
    QCompleter* m_completer;
    FileSystemCompleterModel* m_fileSystemCompleterModel;
};

#endif // ASSETSCONTROLLER_H