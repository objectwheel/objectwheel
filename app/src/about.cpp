#include <about.h>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <fit.h>
#include <QApplication>
#include <flatbutton.h>
#include <scenemanager.h>
#include <usermanager.h>

#define TITLE_TEXT "<p><b>version</b> 1.592 <b>pbuild</b> 529e042<br>Wed May 10 03:32:18 2017 +0300<br></p>"
#define LEGAL_TEXT "<p><b>© 2015 - 2017 Objectwheel, Inc. All Rights Reserved.</b></p>"

using namespace Fit;

struct AboutPrivate
{
		AboutPrivate(QWidget*);
		QWidget* parent;
		QVBoxLayout mainLayout;
		QHBoxLayout iconLayout;
		QLabel iconLabel;
		QLabel titleLabel;
        QLabel legalLabel;
        FlatButton exitButton;
};

AboutPrivate::AboutPrivate(QWidget* p)
	: parent(p)
{
	QPalette palette(parent->palette());
	palette.setColor(QPalette::Window, "#e0e4e7");
	parent->setPalette(palette);

	parent->setAutoFillBackground(true);
	parent->setLayout(&mainLayout);

	iconLabel.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	iconLabel.setFixedSize(fit(150), fit(74.5));
    QPixmap pixmap(":/resources/images/logo.png");
    pixmap.setDevicePixelRatio(qApp->devicePixelRatio());
	iconLabel.setPixmap(pixmap.scaled(fit(150)*qApp->devicePixelRatio(), fit(74.5)*qApp->devicePixelRatio(),
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
    exitButton.setGeometry(parent->width() - fit(26), fit(8), fit(18), fit(18));
#else
    exitButton.setGeometry(parent->width() - fit(15), fit(5), fit(10), fit(10));
#endif
    QObject::connect((About*)parent,  &About::resized, [=]{
#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID) || defined(Q_OS_WINPHONE)
        exitButton.setGeometry(parent->width() - fit(26), fit(8), fit(18), fit(18));
#else
        exitButton.setGeometry(parent->width() - fit(15), fit(5), fit(10), fit(10));
#endif
    });
    fit(&exitButton, Fit::WidthHeight);
    exitButton.show();

    QObject::connect(&exitButton, &FlatButton::clicked, [=]{
        if (UserManager::currentSessionsUser().isEmpty()) {
            SceneManager::show("loginScene", SceneManager::ToRight);
        } else {
            SceneManager::show("studioScene", SceneManager::ToRight);
        }
    });
}

About::About(QWidget *parent)
	: QWidget(parent)
	, m_d(new AboutPrivate(this))
{
}

About::~About()
{
    delete m_d;
}

void About::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    emit resized();
}
