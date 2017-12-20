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

struct AboutWidgetPrivate
{
        AboutWidgetPrivate(QWidget*);
		QWidget* parent;
		QVBoxLayout mainLayout;
		QHBoxLayout iconLayout;
        QLabel topLabel;
		QLabel iconLabel;
		QLabel titleLabel;
        QLabel legalLabel;
        QPushButton okButton;
};

AboutWidgetPrivate::AboutWidgetPrivate(QWidget* p)
	: parent(p)
{
	QPalette palette(parent->palette());
	palette.setColor(QPalette::Window, "#e0e4e7");
	parent->setPalette(palette);
	parent->setAutoFillBackground(true);
	parent->setLayout(&mainLayout);

    okButton.setText("Ok");
    okButton.setFixedWidth(fit::fx(100));
    okButton.setDefault(true);
    QObject::connect(&okButton, SIGNAL(clicked(bool)),
      parent, SLOT(close()));

	iconLabel.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    iconLabel.setFixedSize(fit::fx(150), fit::fx(74.5));
    QPixmap pixmap(":/resources/images/logo.png");
    pixmap.setDevicePixelRatio(pS->devicePixelRatio());
    iconLabel.setPixmap(pixmap.scaled(fit::fx(150) * pS->devicePixelRatio(),
      fit::fx(74.5) * pS->devicePixelRatio(),
      Qt::IgnoreAspectRatio,Qt::SmoothTransformation));

	iconLayout.addStretch();
	iconLayout.addWidget(&iconLabel);
	iconLayout.addStretch();

    QFont f;
    f.setWeight(QFont::ExtraLight);
    f.setPixelSize(fit::fx(24));
    topLabel.setFont(f);
    topLabel.setText("About Objectwheel");
    topLabel.setStyleSheet("background:transparent; color:#2e3a41;");
    topLabel.setAlignment(Qt::AlignCenter);

	titleLabel.setStyleSheet("background:transparent;color:#2e3a41;");
    titleLabel.setText(TEXT_VERSION);
	titleLabel.setAlignment(Qt::AlignCenter);

	legalLabel.setStyleSheet("background:transparent;color:#2e3a41;");
    legalLabel.setText(TEXT_LEGAL);
	legalLabel.setAlignment(Qt::AlignCenter);

    mainLayout.addWidget(&topLabel);
	mainLayout.addStretch();
	mainLayout.addLayout(&iconLayout);
	mainLayout.addWidget(&titleLabel);
    mainLayout.addStretch();
    mainLayout.addWidget(&okButton);
	mainLayout.addStretch();
	mainLayout.addWidget(&legalLabel);
    mainLayout.setAlignment(&okButton, Qt::AlignCenter);

    parent->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint |
      Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);
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

QSize AboutWidget::sizeHint() const
{
    return fit::fx(QSizeF{700, 400}).toSize();
}

