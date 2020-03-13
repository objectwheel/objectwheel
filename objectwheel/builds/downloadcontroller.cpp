#include <downloadcontroller.h>
#include <downloadwidget.h>
#include <utilityfunctions.h>
#include <builddelegate.h>
#include <buildmodel.h>
#include <buildinfo.h>

#include <QLabel>
#include <QListWidget>

DownloadController::DownloadController(DownloadWidget* downloadWidget, QObject* parent)
    : QObject(parent)
    , m_downloadWidget(downloadWidget)
{
    auto noBuildsIndicatorLabel = new QLabel(tr("No builds"), m_downloadWidget->downloadList());
    noBuildsIndicatorLabel->setFixedSize(m_downloadWidget->downloadList()->size());
    noBuildsIndicatorLabel->setAlignment(Qt::AlignCenter);
    noBuildsIndicatorLabel->setStyleSheet("color: #777777");
    UtilityFunctions::adjustFontPixelSize(noBuildsIndicatorLabel, -1);
    connect(m_downloadWidget->downloadList()->model(), &QAbstractItemModel::rowsInserted,
            noBuildsIndicatorLabel, [=] (const QModelIndex& index, int first, int /*last*/) {
        QAbstractItemModel* model = m_downloadWidget->downloadList()->model();
        noBuildsIndicatorLabel->setVisible(model->rowCount() <= 0);
        m_downloadWidget->downloadList()->openPersistentEditor(model->index(first, 0, index));
    });
    connect(m_downloadWidget->downloadList()->model(), &QAbstractItemModel::rowsRemoved,
            noBuildsIndicatorLabel, [=] {
        noBuildsIndicatorLabel->setVisible(m_downloadWidget->downloadList()->model()->rowCount() <= 0);
    });
    connect(m_downloadWidget->downloadList()->model(), &QAbstractItemModel::modelReset,
            noBuildsIndicatorLabel, &QLabel::show);
    connect(static_cast<BuildDelegate*>(m_downloadWidget->downloadList()->itemDelegate()), &BuildDelegate::deleteButtonClicked,
            this, &DownloadController::onDeleteButtonClick);
}

void DownloadController::onDeleteButtonClick(const QModelIndex& index) const
{
    auto model = static_cast<BuildModel*>(m_downloadWidget->downloadList()->model());
    if (BuildInfo* buildInfo = model->buildInfo(index.row())) {
        QMessageBox::StandardButton ret = QMessageBox::Yes;
        if (buildInfo->state() != BuildInfo::Finished) {
            ret = UtilityFunctions::showMessage(m_downloadWidget,
                                                tr("Are you sure?"),
                                                tr("This will cancel the build process."),
                                                QMessageBox::Question,
                                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        }
        if (ret == QMessageBox::Yes)
            model->removeRow(index.row(), index.parent());
    }
}
