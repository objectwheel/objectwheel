#ifndef ASSETSTREE_H
#define ASSETSTREE_H

#include <fileexplorer.h>

class AssetsTree final : public FileExplorer
{
    Q_OBJECT
    Q_DISABLE_COPY(AssetsTree)

public:
    explicit AssetsTree(QWidget* parent = nullptr);
};

#endif // ASSETSTREE_H
