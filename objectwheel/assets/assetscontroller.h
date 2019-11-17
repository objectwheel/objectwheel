#ifndef ASSETSCONTROLLER_H
#define ASSETSCONTROLLER_H

#include <QObject>

class AssetsPane;
class FileSystemCompleterModel;

class AssetsController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AssetsController)

public:
    explicit AssetsController(AssetsPane* assetsPane, QObject* parent = nullptr);

public slots:
    void discharge();

private slots:
    void onModeComboBoxActivation();
    void onSearchEditReturnPress();

private:
    AssetsPane* m_assetsPane;
    FileSystemCompleterModel* m_fileSystemCompleterModel;
};

#endif // ASSETSCONTROLLER_H