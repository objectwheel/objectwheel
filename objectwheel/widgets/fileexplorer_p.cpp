#include <fileexplorer.h>
#include <fileexplorer_p.h>
#include <paintutils.h>
#include <private/qtreeview_p.h>

#include <QPainter>
#include <QLineEdit>
#include <QFileSystemModel>
#include <QFileSystemWatcher>

FileExplorerListDelegate::FileExplorerListDelegate(FileExplorer* parent) : QStyledItemDelegate(parent)
  , m_fileExplorer(parent)
{}

void FileExplorerListDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    const bool isSelected = option.state & QStyle::State_Selected;
    const QAbstractItemModel* model = index.model();
    const QIcon& icon = model->data(index, Qt::DecorationRole).value<QIcon>();

    QRectF iconRect({}, QSizeF{option.decorationSize});
    iconRect.moveCenter(option.rect.center());
    iconRect.moveLeft(option.rect.left() + 5);

    // Fill background
    m_fileExplorer->fillBackground(painter, option, m_fileExplorer->d_func()->viewIndex(index), index.column() == 0);

    // Draw icon
    QPixmap pixmap(PaintUtils::pixmap(icon, option.decorationSize, m_fileExplorer,
                                      isSelected ? QIcon::Selected : QIcon::Normal));
    painter->drawPixmap(iconRect, pixmap, pixmap.rect());

    // Draw text
    if (isSelected)
        painter->setPen(option.palette.highlightedText().color());
    else
        painter->setPen(option.palette.text().color());

    QRectF textRect;
    if (index.column() == 0)
        textRect = option.rect.adjusted(option.decorationSize.width() + 10, 0, 0, 0);
    else
        textRect = option.rect.adjusted(5, 0, 0, 0);

    const QString& text = index.data(Qt::DisplayRole).toString();
    painter->drawText(textRect,
                      option.fontMetrics.elidedText(text, Qt::ElideMiddle, textRect.width()),
                      QTextOption(Qt::AlignLeft | Qt::AlignVCenter));

    painter->restore();
}

PathIndicator::PathIndicator(QWidget* parent) : QWidget(parent)
  , m_pathEdit(new QLineEdit(this))
{
    m_pathEdit->hide();
    m_pathEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    connect(m_pathEdit, &QLineEdit::editingFinished,
            m_pathEdit, &QLineEdit::hide);
    connect(m_pathEdit, &QLineEdit::editingFinished,
            this, [=] { emit pathUpdated(m_pathEdit->text()); });
}

void PathIndicator::setPath(const QString& path)
{
    m_path = path;
    if (m_path == ".")
        m_path = "";
    m_path.prepend("/");
    m_pathEdit->setText(m_path);
    update();
}

void PathIndicator::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), palette().midlight().color());
    painter.setPen(palette().color(QPalette::Shadow));
    painter.drawLine(QRectF(rect()).bottomLeft(), QRectF(rect()).bottomRight());
    painter.setPen(palette().text().color());

    if (m_pathEdit->isHidden()) {
        QTextOption textOption(Qt::AlignLeft | Qt::AlignVCenter);
        textOption.setWrapMode(QTextOption::NoWrap);
        painter.drawText(rect().adjusted(3, 0, -3, 0),
                         fontMetrics().elidedText(m_path, Qt::ElideLeft, width() - 6),
                         textOption);
    }
}

void PathIndicator::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
    m_pathEdit->setGeometry(rect());
}

void PathIndicator::mouseDoubleClickEvent(QMouseEvent* e)
{
    QWidget::mouseDoubleClickEvent(e);
    m_pathEdit->setText(m_path);
    m_pathEdit->show();
    m_pathEdit->setFocus();
}

FileSystemProxyModel::FileSystemProxyModel(QObject* parent) : QSortFilterProxyModel(parent)
{}

bool FileSystemProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    // Sort by "DirsFirst" rule
    if (sortColumn() == 0) {
        const QFileSystemModel* fileSystemModel = qobject_cast<QFileSystemModel*>(sourceModel());

        QFileInfo leftFileInfo  = fileSystemModel->fileInfo(left);
        QFileInfo rightFileInfo = fileSystemModel->fileInfo(right);

        if (leftFileInfo.isDir() && !rightFileInfo.isDir())
            return false;
        if (!leftFileInfo.isDir() && rightFileInfo.isDir())
            return true;

        return QString::localeAwareCompare(leftFileInfo.fileName(), rightFileInfo.fileName()) >= 0;
    }

    return !QSortFilterProxyModel::lessThan(left, right);
}

QVariant FileSystemProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section == 0 && role == Qt::DisplayRole)
        return "  " + tr("Files and Folders");
    if (role == Qt::DecorationRole)
        return QPixmap();
    return QSortFilterProxyModel::headerData(section, orientation, role);
}
