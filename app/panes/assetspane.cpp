#include <assetspane.h>
#include <utilityfunctions.h>
#include <QHeaderView>

namespace {

QPalette initPalette(QWidget* widget)
{
    QPalette palette(widget->palette());
    palette.setColor(QPalette::Light, "#a671bd");
    palette.setColor(QPalette::Dark, "#9968ad");
    palette.setColor(QPalette::AlternateBase, "#f6f2f7");
    palette.setColor(QPalette::ButtonText, "#ffffff");
    palette.setColor(QPalette::WindowText, "#ffffff");
    palette.setColor(QPalette::Mid, palette.dark().color()); // For line color
    palette.setColor(QPalette::Midlight, "#f6f6f6"); // For PathIndicator's background
    palette.setColor(QPalette::Shadow, "#c4c4c4"); // For PathIndicator's border
    return palette;
}
}

AssetsPane::AssetsPane(QWidget* parent) : FileExplorer(parent)
{
    setPalette(initPalette(this));
}

QSize AssetsPane::sizeHint() const
{
    return QSize{310, 250};
}