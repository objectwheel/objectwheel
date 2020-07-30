#include <succeedwidget.h>
#include <buttonslice.h>
#include <utilityfunctions.h>

#include <QMovie>
#include <QBoxLayout>
#include <QPushButton>

enum Buttons { Ok };

SucceedWidget::SucceedWidget(QWidget* parent) : QWidget(parent)
  , m_movie(new QMovie(this))
  , m_titleLabel(new QLabel(this))
  , m_descriptionLabel(new QLabel(this))
{
    auto iconLabel = new QLabel(this);
    iconLabel->setFixedSize(QSize(140, 140));
    iconLabel->setStyleSheet(QStringLiteral("background: transparent"));
    iconLabel->setMovie(m_movie);

    auto buttons = new ButtonSlice(this);
    buttons->add(Ok, QLatin1String("#86CC63"), QLatin1String("#75B257"));
    buttons->get(Ok)->setText(tr("Ok"));
    buttons->get(Ok)->setIcon(QIcon(QStringLiteral(":/images/welcome/ok.png")));
    buttons->get(Ok)->setCursor(Qt::PointingHandCursor);
    buttons->settings().cellWidth = 150;
    buttons->triggerSettings();

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(8);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addStretch();
    layout->addWidget(iconLabel, 0, Qt::AlignHCenter);
    layout->addWidget(m_titleLabel, 0, Qt::AlignHCenter);
    layout->addWidget(m_descriptionLabel, 0, Qt::AlignHCenter);
    layout->addWidget(buttons, 0, Qt::AlignHCenter);
    layout->addStretch();

    UtilityFunctions::adjustFontPixelSize(m_titleLabel, 3);
    m_titleLabel->setStyleSheet(QStringLiteral("color: #65A35C"));
    m_titleLabel->setAlignment(Qt::AlignHCenter);
    m_descriptionLabel->setAlignment(Qt::AlignHCenter);

    m_movie->setFileName(QStringLiteral(":/images/welcome/complete.gif"));
    m_movie->setBackgroundColor(Qt::transparent);
    m_movie->setScaledSize(iconLabel->size() * iconLabel->devicePixelRatioF());
    m_movie->setSpeed(160);

    connect(buttons->get(Ok), &QPushButton::clicked, this, &SucceedWidget::done);
}

void SucceedWidget::play(const QString& title, const QString& description)
{
    m_titleLabel->setText(title);
    m_descriptionLabel->setText(description);
    m_movie->jumpToFrame(0);
    m_movie->start();
}
