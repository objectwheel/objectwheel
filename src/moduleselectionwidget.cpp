#include <moduleselectionwidget.h>
#include <fit.h>
#include <css.h>
#include <filemanager.h>
#include <build.h>

QVBoxLayout* qtScLay,* owScLay;
ModuleSelectionWidget::ModuleSelectionWidget(QWidget *parent)
    : QWidget(parent)
{
    QPalette p(palette());
    p.setColor(backgroundRole(), "#e0e4e7");
    setPalette(p);
    setAutoFillBackground(true);

    QVBoxLayout* layout = new QVBoxLayout(this);
    QHBoxLayout* logoLay = new QHBoxLayout;
    QLabel* lblLogoOw = new QLabel;
    QLabel* lblLogoQt = new QLabel;
    QLabel* lblTitle = new QLabel;
    QLabel* lblMsg = new QLabel;
    QScrollArea* qtScArea = new QScrollArea;
    QGroupBox* qtBox = new QGroupBox;
    QScrollArea* owScArea = new QScrollArea;
    QGroupBox* owBox = new QGroupBox;
    QHBoxLayout* buttonsLay = new QHBoxLayout;
    FlatButton* btnNext = new FlatButton;
    FlatButton* btnBack = new FlatButton;

    layout->setContentsMargins(0, fit::fx(20), 0, fit::fx(20));
    layout->setSpacing(fit::fx(20));
    layout->addLayout(logoLay);
    layout->addWidget(lblTitle);
    layout->addWidget(lblMsg);
    layout->addWidget(qtBox);
    layout->addWidget(owBox);
    layout->addLayout(buttonsLay);
    layout->setAlignment(logoLay, Qt::AlignHCenter);
    layout->setAlignment(lblTitle, Qt::AlignHCenter);
    layout->setAlignment(lblMsg, Qt::AlignHCenter);
    layout->setAlignment(qtBox, Qt::AlignHCenter);
    layout->setAlignment(owBox, Qt::AlignHCenter);
    layout->setAlignment(buttonsLay, Qt::AlignHCenter);

    auto bar = new QWidget;
    bar->setStyleSheet("background: #c0c4c7;");
    bar->setFixedWidth(1);
    bar->setFixedHeight(fit::fx(50));

    logoLay->addWidget(lblLogoOw);
    logoLay->addWidget(bar);
    logoLay->addWidget(lblLogoQt);
    logoLay->setSpacing(fit::fx(20));

    lblLogoQt->setFixedSize(fit::fx(50), fit::fx(50));
    lblLogoQt->setPixmap(QPixmap(":/resources/images/qt.png"));
    lblLogoQt->setScaledContents(true);

    lblLogoOw->setFixedSize(fit::fx(50), fit::fx(50));
    lblLogoOw->setPixmap(QPixmap(":/resources/images/owicon.png"));
    lblLogoOw->setScaledContents(true);

    QFont f;
    f.setPixelSize(fit::fx(28));
    f.setWeight(QFont::ExtraLight);
    QPalette p2(lblTitle->palette());
    p2.setColor(lblTitle->foregroundRole(), "#21303c");
    lblTitle->setFont(f);
    lblTitle->setPalette(p2);
    lblTitle->setText("Module Settings");

    f.setPixelSize(fit::fx(17));
    lblMsg->setFont(f);
    lblMsg->setPalette(p2);
    lblMsg->setText("Select modules you use in your application");

    owBox->setTitle("Qbjectwheel Modules");
    qtBox->setTitle("Qt Modules");

    auto owBoxLay = new QVBoxLayout(owBox);
    owBoxLay->addWidget(owScArea);
    owBoxLay->setContentsMargins(0, 0, 0, 0);
    auto owScWidget = new QWidget;
    owScWidget->setObjectName("owScWidget");
    owScWidget->setStyleSheet("#owScWidget{background: transparent;}");
    owScLay = new QVBoxLayout(owScWidget);
    owScLay->setContentsMargins(0, 0, 0, 0);
    owScArea->setWidget(owScWidget);
    owScArea->setWidgetResizable(true);
    owScArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    owScArea->verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    owScArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    owScArea->setFixedWidth(fit::fx(400));
    owScArea->setStyleSheet("QScrollArea {background: transparent;}");

    auto qtBoxLay = new QVBoxLayout(qtBox);
    qtBoxLay->addWidget(qtScArea);
    qtBoxLay->setContentsMargins(0, 0, 0, 0);
    auto qtScWidget = new QWidget;
    qtScWidget->setObjectName("qtScWidget");
    qtScWidget->setStyleSheet("#qtScWidget{background: transparent;}");
    qtScLay = new QVBoxLayout(qtScWidget);
    qtScLay->setContentsMargins(0, 0, 0, 0);
    qtScArea->setWidget(qtScWidget);
    qtScArea->setWidgetResizable(true);
    qtScArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qtScArea->verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    qtScArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    qtScArea->setFixedWidth(fit::fx(400));
    qtScArea->setStyleSheet("QScrollArea {background: transparent;}");

    QString line;
    QTextStream stream(rdfile(":/resources/other/ow-modules.txt"));
    while (stream.readLineInto(&line)) {
        auto checkbox = new QCheckBox;
        checkbox->setText(line);
        checkbox->setChecked(true);
        owScLay->addWidget(checkbox);
    }

    QTextStream stream2(rdfile(":/resources/other/qt-modules.txt"));
    stream2.readLineInto(&line);
    int dcount = line.toInt();
    while (stream2.readLineInto(&line)) {
        auto checkbox = new QCheckBox;
        checkbox->setText(line);
        checkbox->setChecked(dcount-- > 0);
        checkbox->setDisabled(checkbox->isChecked());
        qtScLay->addWidget(checkbox);
    }

    buttonsLay->addWidget(btnBack);
    buttonsLay->addWidget(btnNext);
    buttonsLay->setSpacing(fit::fx(10));

    btnNext->setColor("#84BF52");
    btnNext->setTextColor(Qt::white);
    btnNext->setFixedSize(fit::fx(200),fit::fx(28));
    btnNext->setRadius(fit::fx(7.5));
    btnNext->setIconSize(QSize(fit::fx(14),fit::fx(14)));
    btnNext->setIcon(QIcon(":/resources/images/load.png"));
    btnNext->setText("Next");
    connect(btnNext, SIGNAL(clicked(bool)),
      SLOT(handleBtnNextClicked()));

    btnBack->setColor("#38A3F6");
    btnBack->setTextColor(Qt::white);
    btnBack->setFixedSize(fit::fx(200),fit::fx(28));
    btnBack->setRadius(fit::fx(7.5));
    btnBack->setIconSize(QSize(fit::fx(14),fit::fx(14)));
    btnBack->setIcon(QIcon(":/resources/images/unload.png"));
    btnBack->setText("Back");
    connect(btnBack, &FlatButton::clicked, [&]{ emit backClicked(); });
}

void ModuleSelectionWidget::handleBtnNextClicked()
{
    for (int i = 0; i < qtScLay->count(); i++) {
        auto checkbox = (QCheckBox*) qtScLay->itemAt(i)->widget();
        Build::setModule(checkbox->text(), checkbox->isChecked(), true);
    }
    for (int i = 0; i < owScLay->count(); i++) {
        auto checkbox = (QCheckBox*) owScLay->itemAt(i)->widget();
        Build::setModule(checkbox->text(), checkbox->isChecked(), false);
    }
    emit done();
}
