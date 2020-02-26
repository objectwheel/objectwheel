#include <downloadwidget.h>
#include <buttonslice.h>
#include <paintutils.h>
#include <buildmodel.h>
#include <builddelegate.h>

#include <QLabel>
#include <QBoxLayout>
#include <QListView>
#include <QScrollBar>
#include <QPushButton>

DownloadWidget::DownloadWidget(QWidget* parent) : QWidget(parent)
  , m_platformList(new QListView(this))
  , m_buttonSlice(new ButtonSlice(this))
{
    m_platformList->setUniformItemSizes(true);
    m_platformList->setIconSize(QSize(54, 54));
    m_platformList->setFixedSize(QSize(600, 400));
    m_platformList->setFocusPolicy(Qt::NoFocus);
    m_platformList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_platformList->setModel(new BuildModel(this));
    m_platformList->setItemDelegate(new BuildDelegate(m_platformList));
    m_platformList->verticalScrollBar()->setStyleSheet(
                QStringLiteral(
                    "QScrollBar:vertical {"
                    "    background: transparent;"
                    "    width: %2px;"
                    "} QScrollBar::handle:vertical {"
                    "    background: #909497;"
                    "    min-height: %1px;"
                    "    border-radius: %3px;"
                    "} QScrollBar::add-line:vertical {"
                    "    background: none;"
                    "} QScrollBar::sub-line:vertical {"
                    "    background: none;"
                    "} QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {"
                    "    background: none;"
                    "} QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
                    "    background: none;"
                    "}").arg(15).arg(6).arg(2.5));
    m_platformList->setStyleSheet(
                QStringLiteral(
                    "QListView {"
                    "    background: #12000000;"
                    "    border: 1px solid #22000000;"
                    "    border-radius: %1px;"
                    "}").arg(8));

    auto iconLabel = new QLabel(this);
    auto titleLabel = new QLabel(tr("Objectwheel Cloud Builds"), this);
    auto descriptionLabel = new QLabel(tr("Downloaded builds appear here"), this);
    auto buildsLabel = new QLabel(tr("Builds"));

    iconLabel->setFixedSize(QSize(60, 60));
    iconLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/builds/gift.svg"),
                                            QSize(60, 60), this));
    QFont f;
    f.setWeight(QFont::ExtraLight);
    f.setPixelSize(26);
    titleLabel->setFont(f);

    f.setWeight(QFont::Light);
    f.setPixelSize(16);
    descriptionLabel->setAlignment(Qt::AlignCenter);
    descriptionLabel->setFont(f);

    m_buttonSlice->add(New, "#8BBB56", "#6EA045");
    m_buttonSlice->get(New)->setText(tr("New"));
    m_buttonSlice->get(New)->setIcon(QIcon(":/images/welcome/new.png"));
    m_buttonSlice->get(New)->setCursor(Qt::PointingHandCursor);
    m_buttonSlice->settings().cellWidth = 150;
    m_buttonSlice->triggerSettings();

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    layout->addWidget(iconLabel, 0, Qt::AlignHCenter);
    layout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    layout->addWidget(descriptionLabel, 0, Qt::AlignHCenter);
    layout->addSpacing(8);
    layout->addWidget(buildsLabel, 0, Qt::AlignHCenter);
    layout->addWidget(m_platformList, 0, Qt::AlignHCenter);
    layout->addWidget(m_buttonSlice, 0, Qt::AlignHCenter);
}

QListView* DownloadWidget::platformList() const
{
    return m_platformList;
}

ButtonSlice* DownloadWidget::buttonSlice() const
{
    return m_buttonSlice;
}
