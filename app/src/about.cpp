#include <about.h>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <fit.h>
#include <QApplication>

#define TITLE_TEXT "<p><b>version</b> 1.588 <b>pbuild</b> ee30820<br>Sat Mar 25 01:00:00 2017 +0300<br></p>"
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
