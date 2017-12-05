#include <aboutwidget.h>
#include <fit.h>
#include <flatbutton.h>
#include <usermanager.h>
#include <global.h>
#include <mainwindow.h>

#include <QApplication>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#define pS (QApplication::primaryScreen())
#define cW (MainWindow::instance()->centralWidget())
#define pW (MainWindow::instance()->progressWidget())
#define TITLE_TEXT "<p><b>version</b> 1.592 <b>pbuild</b> 529e042<br>Wed May 10 03:32:18 2017 +0300<br></p>"
#define LEGAL_TEXT "<p><b>© 2015 - 2017 Objectwheel, Inc. All Rights Reserved.</b></p>"

struct AboutWidgetPrivate
{
        AboutWidgetPrivate(QWidget*);
		QWidget* parent;
		QVBoxLayout mainLayout;
		QHBoxLayout iconLayout;
		QLabel iconLabel;
		QLabel titleLabel;
        QLabel legalLabel;
        FlatButton exitButton;
};

AboutWidgetPrivate::AboutWidgetPrivate(QWidget* p)
	: parent(p)
{
	QPalette palette(parent->palette());
	palette.setColor(QPalette::Window, "#e0e4e7");
	parent->setPalette(palette);

	parent->setAutoFillBackground(true);
	parent->setLayout(&mainLayout);

	iconLabel.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    iconLabel.setFixedSize(fit::fx(150), fit::fx(74.5));
    QPixmap pixmap(":/resources/images/logo.png");
    pixmap.setDevicePixelRatio(pS->devicePixelRatio());
    iconLabel.setPixmap(pixmap.scaled(fit::fx(150) * pS->devicePixelRatio(), fit::fx(74.5) * pS->devicePixelRatio(),
									  Qt::IgnoreAspectRatio,Qt::SmoothTransformation));

	iconLayout.addStretch();
	iconLayout.addWidget(&iconLabel);
	iconLayout.addStretch();

	titleLabel.setStyleSheet("background:transparent;color:#2e3a41;");
	titleLabel.setText(TITLE_TEXT);
	titleLabel.setAlignment(Qt::AlignCenter);

	legalLabel.setStyleSheet("background:transparent;color:#2e3a41;");
	legalLabel.setText(LEGAL_TEXT);
	legalLabel.setAlignment(Qt::AlignCenter);

	mainLayout.addStretch();
	mainLayout.addLayout(&iconLayout);
	mainLayout.addWidget(&titleLabel);
	mainLayout.addStretch();
	mainLayout.addWidget(&legalLabel);

    exitButton.setParent(parent);
    exitButton.setIconButton(true);
    exitButton.setIcon(QIcon(":/resources/images/delete-icon.png"));
    #if defined(Q_OS_IOS) || defined(Q_OS_ANDROID) || defined(Q_OS_WINPHONE)
    exitButton.setGeometry(parent->width() - fit::fx(26), fit::fx(8), fit::fx(18), fit::fx(18));
    #else
    exitButton.setGeometry(parent->width() - fit::fx(15), fit::fx(5), fit::fx(8), fit::fx(8));
    #endif
    QObject::connect((AboutWidget*)parent,  &AboutWidget::resized, [=]{
    #if defined(Q_OS_IOS) || defined(Q_OS_ANDROID) || defined(Q_OS_WINPHONE)
        exitButton.setGeometry(parent->width() - fit::fx(26), fit::fx(8), fit::fx(18), fit::fx(18));
    #else
        exitButton.setGeometry(parent->width() - fit::fx(15), fit::fx(5), fit::fx(8), fit::fx(8));
    #endif
    });
    fit::fx(&exitButton, fit::both);
    exitButton.show();

    QObject::connect(&exitButton, &FlatButton::clicked, [=]{
        if (UserManager::currentSessionsUser().isEmpty()) {
            cW->show(Screen::Login);
        } else {
            cW->show(Screen::Studio);
        }
    });
}

AboutWidget::AboutWidget(QWidget *parent)
	: QWidget(parent)
    , _d(new AboutWidgetPrivate(this))
{
}

AboutWidget::~AboutWidget()
{
    delete _d;
}

void AboutWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    emit resized();
}
