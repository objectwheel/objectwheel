#ifndef ASSETSCONTROLLER_H
#define ASSETSCONTROLLER_H

#include <QObject>

class AssetsPane;

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

private:
    AssetsPane* m_assetsPane;
};

#endif // ASSETSCONTROLLER_H