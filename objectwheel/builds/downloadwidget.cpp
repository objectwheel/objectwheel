#include <downloadwidget.h>
#include <buttonslice.h>
#include <paintutils.h>
#include <buildmodel.h>
#include <builddelegate.h>
#include <utilityfunctions.h>
#include <appconstants.h>

#include <QLabel>
#include <QBoxLayout>
#include <QListView>
#include <QScrollBar>
#include <QPushButton>

DownloadWidget::DownloadWidget(QWidget* parent) : QWidget(parent)
  , m_downloadList(new QListView(this))
  , m_buttonSlice(new ButtonSlice(this))
{
    m_downloadList->setUniformItemSizes(true);
    m_downloadList->setIconSize(QSize(54, 54));
    m_downloadList->setFixedSize(QSize(550, 360));
    m_downloadList->setFocusPolicy(Qt::NoFocus);
    m_downloadList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_downloadList->setModel(new BuildModel(this));
    m_downloadList->setItemDelegate(new BuildDelegate(m_downloadList));
    m_downloadList->verticalScrollBar()->setStyleSheet(
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
    m_downloadList->setStyleSheet(
                QStringLiteral(
                    "QListView {"
                    "    background: #12000000;"
                    "    border: 1px solid #22000000;"
                    "    border-radius: %1px;"
                    "}").arg(8));

    auto iconLabel = new QLabel(this);
    auto titleLabel = new QLabel(tr("%1 Cloud Builds").arg(AppConstants::NAME), this);
    auto descriptionLabel = new QLabel(tr("Recent builds appear here"), this);
    auto buildsLabel = new QLabel(tr("Builds"));

    iconLabel->setFixedSize(QSize(60, 60));
    iconLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/builds/gift.svg"),
                                            QSize(60, 60), this));
    QFont f = UtilityFunctions::systemTitleFont();
    f.setWeight(QFont::Light);
    f.setPixelSize(24);
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
    layout->addWidget(m_downloadList, 0, Qt::AlignHCenter);
    layout->addWidget(m_buttonSlice, 0, Qt::AlignHCenter);
}

QListView* DownloadWidget::downloadList() const
{
    return m_downloadList;
}

ButtonSlice* DownloadWidget::buttonSlice() const
{
    return m_buttonSlice;
}
