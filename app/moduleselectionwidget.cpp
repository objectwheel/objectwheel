#include <moduleselectionwidget.h>
#include <css.h>
#include <filemanager.h>
#include <build.h>
#include <QMap>

QMap<QString, QStringList> deps;
QVBoxLayout* qtScLay,* owScLay;
ModuleSelectionWidget::ModuleSelectionWidget(QWidget *parent)
    : QWidget(parent)
{
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

    layout->setContentsMargins(0, 20, 0, 20);
    layout->setSpacing(20);
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
    bar->setFixedHeight(50);

    logoLay->addWidget(lblLogoOw);
    logoLay->addWidget(bar);
    logoLay->addWidget(lblLogoQt);
    logoLay->setSpacing(20);

    lblLogoQt->setFixedSize(50, 50);
    lblLogoQt->setPixmap(QPixmap(":/images/qt.png"));
    lblLogoQt->setScaledContents(true);

    lblLogoOw->setFixedSize(50, 50);
    lblLogoOw->setPixmap(QPixmap(":/images/owicon.png"));
    lblLogoOw->setScaledContents(true);

    QFont f;
    f.setPixelSize(28);
    f.setWeight(QFont::ExtraLight);
    QPalette p2(lblTitle->palette());
    p2.setColor(lblTitle->foregroundRole(), "#21303c");
    lblTitle->setFont(f);
    lblTitle->setPalette(p2);
    lblTitle->setText("Module Settings");

    f.setPixelSize(17);
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
    owScArea->setFixedWidth(400);
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
    qtScArea->setFixedWidth(400);
    qtScArea->setStyleSheet("QScrollArea {background: transparent;}");

    QString line, lastModule;
    QTextStream stream(rdfile(":/resources/other/ow-modules.txt"));
    while (stream.readLineInto(&line)) {
        if (line.startsWith(" ")) {
            line.remove(0, 1);
            if (deps.contains(lastModule))
                deps[lastModule] << line;
            else
                deps[lastModule] = (QStringList() << line);
        } else {
            lastModule = line;
            auto checkbox = new QCheckBox;
            checkbox->setText(line);
            checkbox->setToolTip(checkbox->text());
            owScLay->addWidget(checkbox);
            connect(checkbox, &QCheckBox::clicked, [=] {
                if (deps.contains(checkbox->text())) {
                    for (int i = 0; i < qtScLay->count(); i++) {
                         auto chk = (QCheckBox*) qtScLay->itemAt(i)->widget();
                         if (deps.value(checkbox->text()).contains(chk->text())) {
                             if (checkbox->isChecked()) {
                                 chk->setChecked(true);
                                 chk->setDisabled(true);
                                 if (!chk->toolTip().contains("Needed by"))
                                     chk->setToolTip(chk->toolTip() + ":");
                                 chk->setToolTip(chk->toolTip() + "\n └ Needed by: " + checkbox->text());
                             } else {
                                 if (chk->toolTip().contains(checkbox->text())) {
                                     chk->setToolTip(chk->toolTip().remove("\n └ Needed by: " + checkbox->text()));
                                     if (!chk->toolTip().contains("Needed by")) {
                                         chk->setChecked(false);
                                         chk->setDisabled(false);
                                         chk->setToolTip(chk->text());
                                     }
                                 }
                             }
                         }
                    }
                }
            });
        }
    }

    QTextStream stream2(rdfile(":/resources/other/qt-modules.txt"));
    stream2.readLineInto(&line);
    int dcount = line.toInt();
    while (stream2.readLineInto(&line)) {
        auto checkbox = new QCheckBox;
        checkbox->setText(line);
        checkbox->setChecked(dcount-- > 0);
        checkbox->setDisabled(checkbox->isChecked());
        if (checkbox->isChecked())
            checkbox->setToolTip(checkbox->text() + ":\n └ Essential");
        else
            checkbox->setToolTip(checkbox->text());
        qtScLay->addWidget(checkbox);
    }

    buttonsLay->addWidget(btnBack);
    buttonsLay->addWidget(btnNext);
    buttonsLay->setSpacing(10);

    btnNext->settings().topColor = "#84BF52";
    btnNext->settings().bottomColor = btnNext->settings().topColor.darker(120);
    btnNext->settings().borderRadius = 7.5;
    btnNext->settings().textColor = Qt::white;
    btnNext->setFixedSize(200,28);
    btnNext->setIconSize(QSize(14,14));
    btnNext->setIcon(QIcon(":/images/load.png"));
    btnNext->setText("Next");
    connect(btnNext, SIGNAL(clicked(bool)),
      SLOT(handleBtnNextClicked()));

    btnBack->settings().topColor = "#38A3F6";
    btnBack->settings().bottomColor = btnBack->settings().topColor.darker(120);
    btnBack->settings().borderRadius = 7.5;
    btnBack->settings().textColor = Qt::white;
    btnBack->setFixedSize(200,28);
    btnBack->setIconSize(QSize(14,14));
    btnBack->setIcon(QIcon(":/images/unload.png"));
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
//TODO: Add "reset form"
