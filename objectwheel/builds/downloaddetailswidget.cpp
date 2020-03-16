#include <downloaddetailswidget.h>
#include <QPainter>

DownloadDetailsWidget::DownloadDetailsWidget(const QAbstractItemView* view, QWidget* parent) : QWidget(parent)
  , m_view(view)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(m_view->model(), &QAbstractItemModel::rowsAboutToBeRemoved,
            this, [=] (const QModelIndex& parent, int first, int last) {
        for (; first <= last; ++first) {
            if (m_index == m_view->model()->index(first, 0, parent)) {
                setIndex(QModelIndex());
                break;
            }
        }
    });
    connect(m_view->model(), &QAbstractItemModel::modelReset,
            this, [=] {
        setIndex(QModelIndex());
    });
    connect(m_view->model(), &QAbstractItemModel::dataChanged,
            this, [=] (const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>&) {
        Q_ASSERT(topLeft == bottomRight);
        if (m_index == topLeft) {
            m_view->itemDelegate()->updateEditorGeometry(m_editor, viewOptions(), m_index);
            updateGeometry();
            update();
        }
    });
}

DownloadDetailsWidget::~DownloadDetailsWidget()
{
    setIndex(QModelIndex());
}

const QModelIndex& DownloadDetailsWidget::index() const
{
    return m_index;
}

void DownloadDetailsWidget::setIndex(const QModelIndex& index)
{
    if (m_index != index) {
        if (m_index.isValid())
            m_view->itemDelegate()->destroyEditor(m_editor, m_index);

        m_index = index;

        if (m_index.isValid()) {
            m_editor = m_view->itemDelegate()->createEditor(this, viewOptions(), m_index);
            m_view->itemDelegate()->updateEditorGeometry(m_editor, viewOptions(), m_index);
        }

        updateGeometry();
        update();
    }
}

QSize DownloadDetailsWidget::sizeHint() const
{
    return minimumSizeHint();
}

QSize DownloadDetailsWidget::minimumSizeHint() const
{
    return m_view->itemDelegate()->sizeHint(viewOptions(), m_index);
}

void DownloadDetailsWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    if (m_index.isValid())
        m_view->itemDelegate()->updateEditorGeometry(m_editor, viewOptions(), m_index);
}

void DownloadDetailsWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    if (m_index.isValid()) {
        m_view->itemDelegate()->paint(&painter, viewOptions(), m_index);
    } else {
        painter.setPen(QColor(0, 0, 0, 130));
        painter.drawText(rect(), tr("No builds"), Qt::AlignVCenter | Qt::AlignHCenter);
    }
    painter.setPen(QPen(QColor("#c4c4c4"), 0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5));
}

QStyleOptionViewItem DownloadDetailsWidget::viewOptions() const
{
    QStyleOptionViewItem option;
    option.initFrom(this);
    option.widget = this;
    option.rect = rect();
    option.decorationSize = m_view->iconSize();
    return option;
}
