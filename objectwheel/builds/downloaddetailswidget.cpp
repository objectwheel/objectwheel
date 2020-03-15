#include <downloaddetailswidget.h>
#include <QPainter>
#include <QAbstractItemView>
#include <QDebug>

DownloadDetailsWidget::DownloadDetailsWidget(const QAbstractItemView* view, QWidget* parent) : QWidget(parent)
  , m_view(view)
{
    connect(m_view->model(), &QAbstractItemModel::rowsRemoved,
            this, [=] (const QModelIndex& parent, int first, int last) {
        if (m_index.isValid()) {
            for (; first <= last; ++first) {
                const QModelIndex& removed = m_view->model()->index(first, 0, parent);
                if (m_index == removed) {
                    m_view->itemDelegate()->destroyEditor(m_editor, m_index);
                    break;
                }
            }
        }
    });
    connect(m_view->model(), &QAbstractItemModel::modelReset,
            this, [=] {
        if (m_index.isValid())
            m_view->itemDelegate()->destroyEditor(m_editor, m_index);
    });
    connect(m_view->model(), &QAbstractItemModel::dataChanged,
            this, [=] {
        if (m_index.isValid()) {
            m_view->itemDelegate()->updateEditorGeometry(m_editor, viewOptions(), m_index);
            updateGeometry();
            update();
        }
    });
}

DownloadDetailsWidget::~DownloadDetailsWidget()
{
    if (m_index.isValid())
        m_view->itemDelegate()->destroyEditor(m_editor, m_index);
}

QSize DownloadDetailsWidget::sizeHint() const
{
    return minimumSizeHint();
}

QSize DownloadDetailsWidget::minimumSizeHint() const
{
    return m_view->itemDelegate()->sizeHint(viewOptions(), m_index);
}

const QModelIndex& DownloadDetailsWidget::index() const
{
    return m_index;
}

void DownloadDetailsWidget::setIndex(const QModelIndex& index)
{
    if (m_index.isValid())
        m_view->itemDelegate()->destroyEditor(m_editor, m_index);

    m_index = index;

    if (m_index.isValid()) {
        m_editor = m_view->itemDelegate()->createEditor(this, viewOptions(), m_index);
        m_view->itemDelegate()->updateEditorGeometry(m_editor, viewOptions(), m_index);
        updateGeometry();
        update();
    }
}

void DownloadDetailsWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_index.isValid())
        m_view->itemDelegate()->updateEditorGeometry(m_editor, viewOptions(), m_index);
}

void DownloadDetailsWidget::paintEvent(QPaintEvent*)
{
    if (m_index.isValid()) {
        QPainter painter(this);
        m_view->itemDelegate()->paint(&painter, viewOptions(), m_index);
    }
}

QStyleOptionViewItem DownloadDetailsWidget::viewOptions() const
{
    QStyleOptionViewItem option;
    option.initFrom(m_view);
    option.widget = m_view;
    option.decorationSize = m_view->iconSize();
    option.rect = rect();
    return option;
}
