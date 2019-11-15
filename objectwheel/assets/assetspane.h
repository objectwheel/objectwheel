#ifndef ASSETSPANE_H
#define ASSETSPANE_H

#include <assetscontroller.h>

class AssetsPane : public FileExplorer
{
    Q_OBJECT

public:
    explicit AssetsPane(QWidget* parent = nullptr);

private:
    QSize sizeHint() const override;
};

#endif // ASSETSPANE_H