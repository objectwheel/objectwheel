#include <projectswidget.h>
#include <buttonslice.h>
#include <fit.h>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QApplication>
#include <QScreen>

#define BUTTONS_WIDTH    (fit::fx(300))
#define SIZE_LOGO        (QSize(fit::fx(80), fit::fx(80)))
#define PATH_LOGO        (":/resources/images/toolbox.png")
#define PATH_NICON       (":/resources/images/new.png")
#define PATH_LICON       (":/resources/images/load.png")
#define PATH_IICON       (":/resources/images/unload.png")
#define pS               (QApplication::primaryScreen())

enum Buttons { Load, New, Import };

ProjectsWidget::ProjectsWidget(QWidget* parent) : QWidget(parent)
{
    _layout = new QVBoxLayout(this);
    _iconLabel = new QLabel;
    _welcomeLabel = new QLabel;
    _versionLabel = new QLabel;
    _projectsLabel = new QLabel;
    _buttons = new ButtonSlice;

    _layout->addStretch();
    _layout->addWidget(_iconLabel, 0, Qt::AlignCenter);
    _layout->addWidget(_welcomeLabel, 0, Qt::AlignCenter);
    _layout->addWidget(_versionLabel, 0, Qt::AlignCenter);
    _layout->addWidget(_projectsLabel, 0, Qt::AlignCenter);
    _layout->addWidget(_buttons, 0, Qt::AlignCenter);
    _layout->addStretch();

    QPixmap p(PATH_LOGO);
    p.setDevicePixelRatio(pS->devicePixelRatio());

    _iconLabel->setFixedSize(SIZE_LOGO);
    _iconLabel->setPixmap(
        p.scaled(
            SIZE_LOGO * pS->devicePixelRatio(),
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        )
    );

    QFont f;
    f.setWeight(QFont::ExtraLight);
    f.setPixelSize(fit::fx(28));

    _welcomeLabel->setFont(f);
    _welcomeLabel->setText(tr("Welcome to Objectwheel"));
    _welcomeLabel->setStyleSheet("color: #2E3A41");

    f.setWeight(QFont::Light);
    f.setPixelSize(fit::fx(15));
    _versionLabel->setFont(f);
    _versionLabel->setText(tr("Version ") + tr(APP_VER) + " (" APP_GITHASH ")");
    _versionLabel->setStyleSheet("color: #2E3A41");

    _projectsLabel->setText(tr("Your Projects"));
    _projectsLabel->setStyleSheet("color: #2E3A41");

    _buttons->add(New, "#CC5D67", "#B2525A");
    _buttons->add(Load, "#5BC5F8", "#2592F9");
    _buttons->add(Import, "#8BBB56", "#6EA045");

    _buttons->get(New)->setText(tr("New"));
    _buttons->get(Load)->setText(tr("Load"));
    _buttons->get(Import)->setText(tr("Import"));

    _buttons->get(New)->setIcon(QIcon(PATH_NICON));
    _buttons->get(Load)->setIcon(QIcon(PATH_LICON));
    _buttons->get(Import)->setIcon(QIcon(PATH_IICON));

    _buttons->get(New)->setCursor(Qt::PointingHandCursor);
    _buttons->get(Load)->setCursor(Qt::PointingHandCursor);
    _buttons->get(Import)->setCursor(Qt::PointingHandCursor);

}
