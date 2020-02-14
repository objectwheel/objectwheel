#include <downloadcontroller.h>
#include <downloadwidget.h>
#include <utilityfunctions.h>

#include <QLabel>
#include <QListWidget>

DownloadController::DownloadController(DownloadWidget* downloadWidget, QObject* parent)
    : QObject(parent)
    , m_downloadWidget(downloadWidget)
{
    auto noBuildsIndicatorLabel = new QLabel(tr("No builds"), m_downloadWidget->platformList());
    noBuildsIndicatorLabel->setFixedSize(m_downloadWidget->platformList()->size());
    noBuildsIndicatorLabel->setAlignment(Qt::AlignCenter);
    noBuildsIndicatorLabel->setStyleSheet("color: #777777");
    UtilityFunctions::adjustFontPixelSize(noBuildsIndicatorLabel, -1);
    connect(m_downloadWidget->platformList()->model(), &QAbstractItemModel::rowsInserted,
            noBuildsIndicatorLabel, [=] {
        noBuildsIndicatorLabel->setVisible(m_downloadWidget->platformList()->count() <= 0);
    });
    connect(m_downloadWidget->platformList()->model(), &QAbstractItemModel::rowsRemoved,
            noBuildsIndicatorLabel, [=] {
        noBuildsIndicatorLabel->setVisible(m_downloadWidget->platformList()->count() <= 0);
    });
    connect(m_downloadWidget->platformList()->model(), &QAbstractItemModel::modelReset,
            noBuildsIndicatorLabel, &QLabel::show);
}
