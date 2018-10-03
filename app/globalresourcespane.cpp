#include <globalresourcespane.h>
#include <projectmanager.h>
#include <saveutils.h>
#include <utilityfunctions.h>

#include <QHeaderView>

namespace {

QPalette initPalette(QWidget* widget)
{
    QPalette palette(widget->palette());
    palette.setColor(QPalette::Light, "#bf5861");
    palette.setColor(QPalette::Dark, "#b05159");
    palette.setColor(QPalette::ButtonText, "#ffffff");
    palette.setColor(QPalette::WindowText, "#ffffff");
    palette.setColor(QPalette::Active, QPalette::Shadow, "#7F393E");
    palette.setColor(QPalette::AlternateBase, "#f7e6e8");
    palette.setColor(QPalette::Mid, palette.dark().color()); // For line color
    palette.setColor(QPalette::Midlight, "#f6f6f6"); // For PathIndicator's background
    palette.setColor(QPalette::Inactive, QPalette::Shadow, "#c4c4c4"); // For PathIndicator's border
    return palette;
}
}

GlobalResourcesPane::GlobalResourcesPane(QWidget* parent) : FileExplorer(parent)
{
    setPalette(initPalette(this));
    UtilityFunctions::adjustFontWeight(header(), QFont::Medium);
    connect(ProjectManager::instance(), &ProjectManager::started,
            this, [=] { setRootPath(SaveUtils::toGlobalDir(ProjectManager::dir())); });
}

QSize GlobalResourcesPane::sizeHint() const
{
    return QSize{310, 250};
}
