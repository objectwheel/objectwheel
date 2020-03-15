#include <downloadcontroller.h>
#include <downloadwidget.h>
#include <utilityfunctions.h>
#include <builddelegate.h>
#include <buildmodel.h>
#include <fileutils.h>

#include <QLabel>
#include <QListWidget>

DownloadController::DownloadController(DownloadWidget* downloadWidget, QObject* parent)
    : QObject(parent)
    , m_downloadWidget(downloadWidget)
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
    connect(delegate, &BuildDelegate::deleteButtonClicked,
            this, &DownloadController::onDeleteButtonClick);
    connect(delegate, &BuildDelegate::openFolderButtonClicked,
            this, &DownloadController::onOpenFolderButtonClick);
    connect(model, &BuildModel::dataChanged,
            this, &DownloadController::onModelDataChange);
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

void DownloadController::onModelDataChange(const QModelIndex& topLeft,
                                           const QModelIndex& bottomRight,
                                           const QVector<int>& roles) const
{
    if (roles.isEmpty()
            || roles.contains(Qt::ToolTipRole)
            || roles.contains(Qt::WhatsThisRole)
            || roles.contains(Qt::StatusTipRole)) {
        Q_ASSERT(topLeft == bottomRight);
        const QString& toolTip = topLeft.data(Qt::ToolTipRole).toString();
        QListView* listView = m_downloadWidget->downloadList();
        UtilityFunctions::updateToolTip(listView, toolTip, listView->visualRect(topLeft));
    }
}
