#include <downloaddetailswidget.h>
#include <buildmodel.h>
#include <QPainter>

DownloadDetailsWidget::DownloadDetailsWidget(const QAbstractItemView* view, QWidget* parent) : QWidget(parent)
  , m_view(view)
{
    Q_ASSERT(m_view);
    Q_ASSERT(m_view->model());
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(m_view->model(), &QAbstractItemModel::rowsAboutToBeRemoved,
            this, &DownloadDetailsWidget::onRowsAboutToBeRemoved);
    connect(m_view->model(), &QAbstractItemModel::modelReset,
            this, &DownloadDetailsWidget::onModelReset);
    connect(m_view->model(), &QAbstractItemModel::dataChanged,
            this, &DownloadDetailsWidget::onDataChange);
}

DownloadDetailsWidget::~DownloadDetailsWidget()
{
    setIdentifier(QByteArray());
}

QByteArray DownloadDetailsWidget::identifier() const
{
    return m_identifier;
}

void DownloadDetailsWidget::setIdentifier(const QByteArray& identifier)
{
    if (m_identifier != identifier) {
        if (index().isValid() && m_view->itemDelegate())
            m_view->itemDelegate()->destroyEditor(m_editor, index());

        m_identifier = identifier;

        if (index().isValid() && m_view->itemDelegate()) {
            m_editor = m_view->itemDelegate()->createEditor(this, viewOptions(), index());
            m_view->itemDelegate()->updateEditorGeometry(m_editor, viewOptions(), index());
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
    return m_view->itemDelegate() ? m_view->itemDelegate()->sizeHint(viewOptions(), index()) : QSize();
}

void DownloadDetailsWidget::onModelReset()
{
    setIdentifier(QByteArray());
}

void DownloadDetailsWidget::onRowsAboutToBeRemoved(const QModelIndex&, int first, int last)
{
    for (; first <= last; ++first) {
        if (first == index().row()) {
            setIdentifier(QByteArray());
            break;
        }
    }
}

void DownloadDetailsWidget::onDataChange(const QModelIndex& topLeft, const QModelIndex& bottomRight,
                                         const QVector<int>& /*roles*/)
{
    Q_ASSERT(topLeft == bottomRight);
    if (index() == topLeft && m_view->itemDelegate()) {
        m_view->itemDelegate()->updateEditorGeometry(m_editor, viewOptions(), index());
        updateGeometry();
        update();
    }
}

QModelIndex DownloadDetailsWidget::index() const
{
    return m_view->model() ? static_cast<BuildModel*>(m_view->model())->indexFromIdentifier(m_identifier) : QModelIndex();
}

void DownloadDetailsWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    if (index().isValid() && m_view->itemDelegate())
        m_view->itemDelegate()->updateEditorGeometry(m_editor, viewOptions(), index());
}

void DownloadDetailsWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    if (index().isValid() && m_view->itemDelegate()) {
        m_view->itemDelegate()->paint(&painter, viewOptions(), index());
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
