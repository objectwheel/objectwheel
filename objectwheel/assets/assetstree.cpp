#include <assetstree.h>

AssetsTree::AssetsTree(QWidget* parent) : FileExplorer(parent)
{
    auto updatePalette = [=] {
        QPalette p(palette());
        p.setColor(QPalette::Light, "#a671bd");
        p.setColor(QPalette::Dark, "#9968ad");
        p.setColor(QPalette::AlternateBase, "#f6f2f7");
        p.setColor(QPalette::ButtonText, "#ffffff");
        p.setColor(QPalette::WindowText, "#ffffff");
        p.setColor(QPalette::Mid, p.dark().color()); // For line color
        p.setColor(QPalette::Midlight, "#f6f6f6"); // For PathIndicator's background
        p.setColor(QPalette::Shadow, "#c4c4c4"); // For PathIndicator's border
        setPalette(p);
    };
    connect(qApp, &QApplication::paletteChanged, this, updatePalette);
    updatePalette();
}