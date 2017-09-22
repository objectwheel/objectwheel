#include <fileexplorer.h>
#include <fit.h>

#define SIZE_HINT (QSize(fit(150), fit(300)))

using namespace Fit;

FileExplorer::FileExplorer(QWidget *parent) : QWidget(parent)
{
    setAutoFillBackground(true);
    QPalette p(palette());
    p.setColor(QPalette::Background, Qt::red);
    setPalette(p);
}

QSize FileExplorer::sizeHint() const
{
    return SIZE_HINT;
}
