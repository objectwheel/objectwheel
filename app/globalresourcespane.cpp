#include <globalresourcespane.h>
#include <projectmanager.h>
#include <saveutils.h>

#include <QHeaderView>

namespace {

QPalette initPalette(QWidget* widget)
{
    QPalette palette(widget->palette());
    palette.setColor(QPalette::Light, "#bf5861");
    palette.setColor(QPalette::Dark, "#b05159");
    palette.setColor(QPalette::Active, QPalette::Shadow, "#7F393E");
    palette.setColor(QPalette::Base, Qt::white);
    palette.setColor(QPalette::Text, "#331719");
    palette.setColor(QPalette::BrightText, Qt::white);
    palette.setColor(QPalette::WindowText, "#331719");
    palette.setColor(QPalette::AlternateBase, "#f7e6e8");
    palette.setColor(QPalette::Midlight, "#f6f6f6"); // For PathIndicator's background
    palette.setColor(QPalette::Inactive, QPalette::Shadow, "#c4c4c4"); // For PathIndicator's border
    return palette;
}
}

GlobalResourcesPane::GlobalResourcesPane(QWidget* parent) : FileExplorer(parent)
{
    setPalette(initPalette(this));
    QFont fontMedium(font());
    fontMedium.setWeight(QFont::Medium);
    header()->setFont(fontMedium);
    connect(ProjectManager::instance(), &ProjectManager::started,
            this, [=] { setRootPath(SaveUtils::toGlobalDir(ProjectManager::dir())); });
}

QSize GlobalResourcesPane::sizeHint() const
{
    return QSize{310, 280};
}
