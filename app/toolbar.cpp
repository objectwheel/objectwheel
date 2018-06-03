#include <toolbar.h>

#include <utils/theme/theme.h>

#include <QPainter>
#include <QBoxLayout>
#include <QLabel>

class SeparatorWidget : public QWidget {
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.fillRect(rect(), Utils::creatorTheme()->color(Utils::Theme::FancyToolBarSeparatorColor));
    }
public:
    explicit SeparatorWidget(ToolBar* parent = nullptr) : QWidget(parent) {
        if (parent->orientation() == Qt::Horizontal)
            setFixedSize({1, 18});
        else
            setFixedSize({18, 1});

        connect(parent, &ToolBar::orientationChanged, this, [=] (Qt::Orientation o) {
            if (o == Qt::Horizontal)
                setFixedSize({1, 18});
            else
                setFixedSize({18, 1});
        });
    }
};

ToolBar::ToolBar(QWidget* parent) : QWidget(parent)
  , m_layout(new QBoxLayout(QBoxLayout::LeftToRight, this))
{
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    setOrientation(Qt::Horizontal);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void ToolBar::addStretch()
{
    m_layout->addStretch();
}

void ToolBar::addSeparator()
{
    m_layout->addWidget(new SeparatorWidget(this));
}

void ToolBar::addSpacing(int spacing)
{
    m_layout->addSpacing(spacing);
}

void ToolBar::addWidget(QWidget* widget)
{
    m_layout->addWidget(widget);
}

void ToolBar::setOrientation(Qt::Orientation orientation)
{
    if (orientation == Qt::Horizontal)
        m_layout->setDirection(QBoxLayout::LeftToRight);
    else
        m_layout->setDirection(QBoxLayout::TopToBottom);
    emit orientationChanged(orientation);
}

Qt::Orientation ToolBar::orientation() const
{
    if (m_layout->direction() == QBoxLayout::LeftToRight)
        return Qt::Horizontal;
    else
        return Qt::Vertical;
}

QSize ToolBar::sizeHint() const
{
    return {24, 24};
}

void ToolBar::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.fillRect(rect(), Utils::creatorTheme()->color(Utils::Theme::ToolBarBackgroundColor));
}
