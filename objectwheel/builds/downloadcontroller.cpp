#include <downloadcontroller.h>
#include <downloadwidget.h>
#include <utilityfunctions.h>
#include <builddelegate.h>
#include <buildmodel.h>
#include <fileutils.h>
#include <builddetailsdialog.h>

#include <QLabel>
#include <QListWidget>

DownloadController::DownloadController(DownloadWidget* downloadWidget, QObject* parent) : QObject(parent)
  , m_downloadWidget(downloadWidget)
  , m_buildDetailsDialog(new BuildDetailsDialog(m_downloadWidget->downloadList(), m_downloadWidget->downloadList()))
{
    auto model = static_cast<const BuildModel*>(m_downloadWidget->downloadList()->model());
    auto delegate = static_cast<BuildDelegate*>(m_downloadWidget->downloadList()->itemDelegate());
    auto noBuildsIndicatorLabel = new QLabel(tr("No builds"), m_downloadWidget->downloadList());
    noBuildsIndicatorLabel->setFixedSize(m_downloadWidget->downloadList()->size());
    noBuildsIndicatorLabel->setAlignment(Qt::AlignCenter);
    noBuildsIndicatorLabel->setStyleSheet("color: #777777");
    UtilityFunctions::adjustFontPixelSize(noBuildsIndicatorLabel, -1);
    connect(model, &BuildModel::rowsInserted,
            noBuildsIndicatorLabel, [=] (const QModelIndex& index, int first, int /*last*/) {
        noBuildsIndicatorLabel->setVisible(model->rowCount() <= 0);
        m_downloadWidget->downloadList()->openPersistentEditor(model->index(first, 0, index));
    });
    connect(model, &BuildModel::rowsRemoved,
            noBuildsIndicatorLabel, [=] {
        noBuildsIndicatorLabel->setVisible(model->rowCount() <= 0);
    });
    connect(model, &BuildModel::modelReset,
            noBuildsIndicatorLabel, &QLabel::show);
    connect(delegate, &BuildDelegate::infoButtonClicked,
            this, &DownloadController::onInfoButtonClick);
    connect(delegate, &BuildDelegate::deleteButtonClicked,
            this, &DownloadController::onDeleteButtonClick);
    connect(delegate, &BuildDelegate::openFolderButtonClicked,
            this, &DownloadController::onOpenFolderButtonClick);
}

void DownloadController::onInfoButtonClick(const QModelIndex& index) const
{
    m_buildDetailsDialog->setIdentifier(index.data(BuildModel::Identifier).toByteArray());
    m_buildDetailsDialog->exec();
}

void DownloadController::onDeleteButtonClick(const QModelIndex& index) const
{
    auto model = static_cast<BuildModel*>(m_downloadWidget->downloadList()->model());
    const QVariant& state = model->data(index, BuildModel::StateRole);
    if (state.isValid()) {
        QMessageBox::StandardButton ret = QMessageBox::Yes;
        if (state.toInt() != BuildModel::Finished) {
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

void DownloadController::onOpenFolderButtonClick(const QModelIndex& index) const
{
    auto model = static_cast<const BuildModel*>(m_downloadWidget->downloadList()->model());
    const QVariant& state = model->data(index, BuildModel::PathRole);
    if (state.isValid())
        Utils::FileUtils::showInFolder(m_downloadWidget, state.toString());
}
