#include <fileexplorer.h>
#include <fit.h>

#define SIZE_HINT (QSize(fit(50), fit(300)))

using namespace Fit;

FileExplorer::FileExplorer(QWidget *parent) : QWidget(parent)
{

}

QSize FileExplorer::sizeHint() const
{
    return SIZE_HINT;
}
