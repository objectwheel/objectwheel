#include <about.h>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <fit.h>

#define TITLE_TEXT "<p><b>version</b> 1.53 <b>pbuild</b> dddd759<br>Fri Dec 30 05:00:20 2016 +0300<br></p>"
#define LEGAL_TEXT "<p><b>© 2015-2016 Objectwheel, Inc. All Rights Reserved.</b></p>"

using namespace Fit;

struct AboutPrivate
{
		AboutPrivate(QWidget*, QWidget*);
		void show(bool show);

		QWidget* parent;
		QWidget* centralWidget;
		QParallelAnimationGroup parallelAnimationGroup;
		QPropertyAnimation aboutShifterAmination;
		QPropertyAnimation cwShifterAmination;
		QMetaObject::Connection connection;

		QVBoxLayout mainLayout;
		QHBoxLayout iconLayout;
		QLabel iconLabel;
		QLabel titleLabel;
		QLabel legalLabel;
};

AboutPrivate::AboutPrivate(QWidget* p, QWidget* cW)
	: parent(p)
	, centralWidget(cW)
{
	parallelAnimationGroup.addAnimation(&cwShifterAmination);
	parallelAnimationGroup.addAnimation(&aboutShifterAmination);

	QPalette palette(parent->palette());
	palette.setColor(QPalette::Window,"#444444");
	parent->setPalette(palette);

	parent->setAutoFillBackground(true);
	parent->setGeometry(QRect(centralWidget->width(), 0, centralWidget->width(), centralWidget->height()));
	parent->setLayout(&mainLayout);

	iconLabel.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	iconLabel.setFixedSize(fit(150), fit(74.5));
	iconLabel.setPixmap(QPixmap(":/resources/images/logo.png").
						scaled(fit(150), fit(74.5),Qt::IgnoreAspectRatio,Qt::SmoothTransformation));

	iconLayout.addStretch();
	iconLayout.addWidget(&iconLabel);
	iconLayout.addStretch();

	titleLabel.setStyleSheet("background:transparent;color:white;");
	titleLabel.setText(TITLE_TEXT);
	titleLabel.setAlignment(Qt::AlignCenter);

	legalLabel.setStyleSheet("background:transparent;color:white;");
	legalLabel.setText(LEGAL_TEXT);
	legalLabel.setAlignment(Qt::AlignCenter);

	mainLayout.addStretch();
	mainLayout.addLayout(&iconLayout);
	mainLayout.addWidget(&titleLabel);
	mainLayout.addStretch();
	mainLayout.addWidget(&legalLabel);
}

void AboutPrivate::show(bool show)
{
	if (show) {
		cwShifterAmination.stop();
		aboutShifterAmination.stop();
		parallelAnimationGroup.stop();

		QObject::disconnect(connection);

		cwShifterAmination.setTargetObject(centralWidget);
		cwShifterAmination.setPropertyName("geometry");
		cwShifterAmination.setDuration(500);
		cwShifterAmination.setEasingCurve(QEasingCurve::OutExpo);
		cwShifterAmination.setStartValue(QRect(0, 0, ((QWidget*)parent->parent())->width(), ((QWidget*)parent->parent())->height()));
		cwShifterAmination.setEndValue(QRect(-((QWidget*)parent->parent())->width(), 0, ((QWidget*)parent->parent())->width(), ((QWidget*)parent->parent())->height()));

		aboutShifterAmination.setTargetObject(parent);
		aboutShifterAmination.setPropertyName("geometry");
		aboutShifterAmination.setDuration(500);
		aboutShifterAmination.setEasingCurve(QEasingCurve::OutExpo);
		aboutShifterAmination.setStartValue(QRect(((QWidget*)parent->parent())->width(), 0, ((QWidget*)parent->parent())->width(), ((QWidget*)parent->parent())->height()));
		aboutShifterAmination.setEndValue(QRect(0, 0, ((QWidget*)parent->parent())->width(), ((QWidget*)parent->parent())->height()));

		parallelAnimationGroup.start();
		parent->show();
	} else {
		cwShifterAmination.stop();
		aboutShifterAmination.stop();
		parallelAnimationGroup.stop();

		cwShifterAmination.setTargetObject(centralWidget);
		cwShifterAmination.setPropertyName("geometry");
		cwShifterAmination.setDuration(500);
		cwShifterAmination.setEasingCurve(QEasingCurve::OutExpo);
		cwShifterAmination.setStartValue(QRect(-((QWidget*)parent->parent())->width(), 0, ((QWidget*)parent->parent())->width(), ((QWidget*)parent->parent())->height()));
		cwShifterAmination.setEndValue(QRect(0, 0, ((QWidget*)parent->parent())->width(), ((QWidget*)parent->parent())->height()));

		aboutShifterAmination.setTargetObject(parent);
		aboutShifterAmination.setPropertyName("geometry");
		aboutShifterAmination.setDuration(500);
		aboutShifterAmination.setEasingCurve(QEasingCurve::OutExpo);
		aboutShifterAmination.setStartValue(QRect(0, 0, ((QWidget*)parent->parent())->width(), ((QWidget*)parent->parent())->height()));
		aboutShifterAmination.setEndValue(QRect(((QWidget*)parent->parent())->width(), 0, ((QWidget*)parent->parent())->width(), ((QWidget*)parent->parent())->height()));

		parallelAnimationGroup.start();
		connection = QObject::connect(&parallelAnimationGroup, &QParallelAnimationGroup::finished, [&]{parent->hide();});
	}
}

About::About(QWidget* centralWidget, QWidget *parent)
	: QWidget(parent)
	, m_d(new AboutPrivate(this, centralWidget))
{
	setHidden(true);
}

About::~About()
{
	delete m_d;
}

void About::show(bool show)
{
	m_d->show(show);
}
