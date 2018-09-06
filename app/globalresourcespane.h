#ifndef GLOBALRESOURCESPANE_H
#define GLOBALRESOURCESPANE_H

#include <fileexplorer.h>

class GlobalResourcesPane : public FileExplorer
{
    Q_OBJECT

public:
    explicit GlobalResourcesPane(QWidget* parent = nullptr);

private:
    QSize sizeHint() const override;
};

#endif // GLOBALRESOURCESPANE_H