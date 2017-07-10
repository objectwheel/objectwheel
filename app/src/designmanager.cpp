#include <designmanager.h>
#include <designerscene.h>
#include <designerview.h>
#include <control.h>
#include <qmlpreviewer.h>
#include <fit.h>
#include <css.h>

#include <QWidget>
#include <QList>
#include <QVBoxLayout>
#include <QTimer>
#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QToolButton>
#include <QMenu>
#include <QComboBox>

using namespace Fit;

class DesignManagerPrivate : public QObject
{
        Q_OBJECT
    public:
        DesignManagerPrivate(DesignManager* parent);
        ~DesignManagerPrivate();

    private:
        qreal roundRatio(qreal ratio);
        qreal findRatio(const QString& text);
        QString findText(qreal ratio);
        void scaleScene(qreal ratio);

    private slots:
        void handleShowOutlineAction(bool value);
        void handleFitInSceneAction();
        void handleZoomLevelChange(const QString& text);

    public:
        DesignManager* parent;
        QWidget* settleWidget = nullptr;
        QVBoxLayout layout;
        DesignerScene designerScene;
        DesignerView designerView;
        QmlPreviewer qmlPreviewer;
        qreal lastScale;
        QToolBar toolbar;
        QToolButton refreshPreviewButton;
        QToolButton clearPageButton;
        QToolButton undoButton;
        QToolButton redoButton;
        QToolButton phonePortraitButton;
        QToolButton phoneLandscapeButton;
        QToolButton desktopSkinButton;
        QToolButton noSkinButton;
        QToolButton showOutlineButton;
        QToolButton fitInSceneButton;
        QComboBox zoomlLevelCombobox;
        QToolButton layItVertButton;
        QToolButton layItHorzButton;
        QToolButton layItGridButton;
        QToolButton breakLayoutButton;
};

DesignManagerPrivate::DesignManagerPrivate(DesignManager* parent)
    : parent(parent)
    , designerView(&designerScene)
    , lastScale(1.0)
{
    layout.setContentsMargins(0, 0, 0, 0);
    layout.setSpacing(0);
    layout.addWidget(&toolbar);
    layout.addWidget(&designerView);

    designerView.setRenderHint(QPainter::Antialiasing);
    designerView.setRubberBandSelectionMode(Qt::IntersectsItemShape);
    designerView.setDragMode(QGraphicsView::RubberBandDrag);
    designerView.setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    designerView.setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    designerView.setBackgroundBrush(QColor("#e0e4e7"));
    designerView.setFrameShape(QFrame::NoFrame);

    designerScene.setSceneRect(designerView.rect().adjusted(- designerView.width() / 2.0,
                                                            - designerView.height() / 2.0, 0, 0));

    QTimer::singleShot(3000, [this] {
        auto page = new Page;
        page->setId("applicationWindow");
        page->setUrl(QUrl("qrc:/resources/qmls/mainPage.qml"));
        page->refresh();
        designerScene.addPage(page);
    });

    zoomlLevelCombobox.addItem("10 %");
    zoomlLevelCombobox.addItem("25 %");
    zoomlLevelCombobox.addItem("50 %");
    zoomlLevelCombobox.addItem("75 %");
    zoomlLevelCombobox.addItem("90 %");
    zoomlLevelCombobox.addItem("100 %");
    zoomlLevelCombobox.addItem("125 %");
    zoomlLevelCombobox.addItem("150 %");
    zoomlLevelCombobox.addItem("175 %");
    zoomlLevelCombobox.addItem("200 %");
    zoomlLevelCombobox.addItem("300 %");
    zoomlLevelCombobox.addItem("500 %");
    zoomlLevelCombobox.addItem("1000 %");
    zoomlLevelCombobox.setCurrentIndex(5);

    toolbar.setStyleSheet(CSS::DesignerToolbar);
    toolbar.setIconSize(QSize(fit(14), fit(14)));
    toolbar.setFixedHeight(fit(26));

    showOutlineButton.setCheckable(true);
    showOutlineButton.setChecked(Control::showOutline());

    phonePortraitButton.setCheckable(true);
    phonePortraitButton.setChecked(true);
    phoneLandscapeButton.setCheckable(true);
    desktopSkinButton.setCheckable(true);
    noSkinButton.setCheckable(true);

    refreshPreviewButton.setCursor(Qt::PointingHandCursor);
    clearPageButton.setCursor(Qt::PointingHandCursor);
    undoButton.setCursor(Qt::PointingHandCursor);
    redoButton.setCursor(Qt::PointingHandCursor);
    phonePortraitButton.setCursor(Qt::PointingHandCursor);
    phoneLandscapeButton.setCursor(Qt::PointingHandCursor);
    desktopSkinButton.setCursor(Qt::PointingHandCursor);
    noSkinButton.setCursor(Qt::PointingHandCursor);
    showOutlineButton.setCursor(Qt::PointingHandCursor);
    fitInSceneButton.setCursor(Qt::PointingHandCursor);
    zoomlLevelCombobox.setCursor(Qt::PointingHandCursor);
    layItVertButton.setCursor(Qt::PointingHandCursor);
    layItHorzButton.setCursor(Qt::PointingHandCursor);
    layItGridButton.setCursor(Qt::PointingHandCursor);
    breakLayoutButton.setCursor(Qt::PointingHandCursor);

    refreshPreviewButton.setToolTip("Refresh control previews on the Dashboard.");
    clearPageButton.setToolTip("Clear controls on the Dashboard.");
    undoButton.setToolTip("Undo action.");
    redoButton.setToolTip("Redo action.");
    phonePortraitButton.setToolTip("Skin: Phone portrait.");
    phoneLandscapeButton.setToolTip("Skin: Phone landscape.");
    desktopSkinButton.setToolTip("Skin: Desktop.");
    noSkinButton.setToolTip("No skin (free).");
    showOutlineButton.setToolTip("Show outline frame for controls.");
    fitInSceneButton.setToolTip("Fit scene into the Dashboard.");
    zoomlLevelCombobox.setToolTip("Change zoom level.");
    layItVertButton.setToolTip("Lay out selected controls vertically.");
    layItHorzButton.setToolTip("Lay out selected controls horizontally.");
    layItGridButton.setToolTip("Lay out selected controls as grid.");
    breakLayoutButton.setToolTip("Break layouts for selected controls.");

    refreshPreviewButton.setIcon(QIcon(":/resources/images/refresh.png"));
    clearPageButton.setIcon(QIcon(":/resources/images/clean.png"));
    undoButton.setIcon(QIcon(":/resources/images/undo.png"));
    redoButton.setIcon(QIcon(":/resources/images/redo.png"));
    phonePortraitButton.setIcon(QIcon(":/resources/images/portrait.png"));
    phoneLandscapeButton.setIcon(QIcon(":/resources/images/landscape.png"));
    desktopSkinButton.setIcon(QIcon(":/resources/images/desktop.png"));
    noSkinButton.setIcon(QIcon(":/resources/images/free.png"));
    showOutlineButton.setIcon(QIcon(":/resources/images/outline.png"));
    fitInSceneButton.setIcon(QIcon(":/resources/images/fit.png"));
    layItVertButton.setIcon(QIcon(":/resources/images/vert.png"));
    layItHorzButton.setIcon(QIcon(":/resources/images/hort.png"));
    layItGridButton.setIcon(QIcon(":/resources/images/grid.png"));
    breakLayoutButton.setIcon(QIcon(":/resources/images/break.png"));

    connect(&showOutlineButton, SIGNAL(toggled(bool)), SLOT(handleShowOutlineAction(bool)));
    connect(&zoomlLevelCombobox, SIGNAL(currentTextChanged(QString)), SLOT(handleZoomLevelChange(QString)));
    connect(&fitInSceneButton, SIGNAL(clicked(bool)), SLOT(handleFitInSceneAction()));

    toolbar.addWidget(&undoButton);
    toolbar.addWidget(&redoButton);
    toolbar.addSeparator();
    toolbar.addWidget(&refreshPreviewButton);
    toolbar.addWidget(&clearPageButton);
    toolbar.addSeparator();
    toolbar.addWidget(&phonePortraitButton);
    toolbar.addWidget(&phoneLandscapeButton);
    toolbar.addWidget(&desktopSkinButton);
    toolbar.addWidget(&noSkinButton);
    toolbar.addSeparator();
    toolbar.addWidget(&showOutlineButton);
    toolbar.addWidget(&fitInSceneButton);
    toolbar.addWidget(&zoomlLevelCombobox);

    QWidget* empty = new QWidget();
    empty->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolbar.addWidget(empty);

    toolbar.addWidget(&layItVertButton);
    toolbar.addWidget(&layItHorzButton);
    toolbar.addWidget(&layItGridButton);
    toolbar.addWidget(&breakLayoutButton);
}

DesignManagerPrivate::~DesignManagerPrivate()
{
    /* delete stuff */
}

qreal DesignManagerPrivate::roundRatio(qreal ratio)
{
    if (ratio < 0.1)
        return 0.1;
    else if (ratio >= 0.1 && ratio < 0.25)
        return 0.1;
    else if (ratio >= 0.25 && ratio < 0.5)
        return 0.25;
    else if (ratio >= 0.5 && ratio < 0.75)
        return 0.5;
    else if (ratio >= 0.75 && ratio < 0.9)
        return 0.75;
    else if (ratio >= 0.9 && ratio < 1.0)
        return 0.9;
    else if (ratio >= 1.0 && ratio < 1.25)
        return 1.0;
    else if (ratio >= 1.25 && ratio < 1.5)
        return 1.25;
    else if (ratio >= 1.5 && ratio < 1.75)
        return 1.5;
    else if (ratio >= 1.75 && ratio < 2.0)
        return 1.75;
    else if (ratio >= 2.0 && ratio < 3.0)
        return 2.0;
    else if (ratio >= 3.0 && ratio < 5.0)
        return 3.0;
    else if (ratio >= 5.0 && ratio < 10.0)
        return 5.0;
    else
        return 10.0;
}

qreal DesignManagerPrivate::findRatio(const QString& text)
{
    if (text == "10 %")
        return 0.1;
    else if (text == "25 %")
        return 0.25;
    else if (text == "50 %")
        return 0.50;
    else if (text == "75 %")
        return 0.75;
    else if (text == "90 %")
        return 0.90;
    else if (text == "100 %")
        return 1.0;
    else if (text == "125 %")
        return 1.25;
    else if (text == "150 %")
        return 1.50;
    else if (text == "175 %")
        return 1.75;
    else if (text == "200 %")
        return 2.0;
    else if (text == "300 %")
        return 3.0;
    else if (text == "500 %")
        return 5.0;
    else if (text == "1000 %")
        return 10.0;
    else
        return 1.0;
}

QString DesignManagerPrivate::findText(qreal ratio)
{
    if (ratio == 0.1)
        return "10 %";
    else if (ratio == 0.25)
        return "25 %";
    else if (ratio == 0.50)
        return "50 %";
    else if (ratio == 0.75)
        return "75 %";
    else if (ratio == 0.90)
        return "90 %";
    else if (ratio == 1.0)
        return "100 %";
    else if (ratio == 1.25)
        return "125 %";
    else if (ratio == 1.50)
        return "150 %";
    else if (ratio == 1.75)
        return "175 %";
    else if (ratio == 2.0)
        return "200 %";
    else if (ratio == 3.0)
        return "300 %";
    else if (ratio == 5.0)
        return "500 %";
    else if (ratio == 10.0)
        return "1000 %";
    else
        return "100 %";
}

void DesignManagerPrivate::scaleScene(qreal ratio)
{
    designerView.scale((1.0 / lastScale) * ratio, (1.0 / lastScale) * ratio);
    lastScale = ratio;
}

void DesignManagerPrivate::handleShowOutlineAction(bool value)
{
    designerScene.setShowOutlines(value);
}

void DesignManagerPrivate::handleFitInSceneAction()
{
    auto ratios = { 0.1, 0.25, 0.5, 0.75, 0.9, 1.0, 1.25, 1.50, 1.75, 2.0, 3.0, 5.0, 10.0 };
    auto diff = designerView.width() / designerScene.width();
    for (auto ratio : ratios) {
        if (roundRatio(diff) == ratio) {
            auto itemText = findText(ratio);
            for (int i = 0; i < zoomlLevelCombobox.count(); i++) {
                if (zoomlLevelCombobox.itemText(i) == itemText) {
                    zoomlLevelCombobox.setCurrentIndex(i);
                    break;
                }
            }
            break;
        }
    }
}

void DesignManagerPrivate::handleZoomLevelChange(const QString& text)
{
    qreal ratio = findRatio(text);
    scaleScene(ratio);
}

DesignManagerPrivate* DesignManager::_d = nullptr;

DesignManager::DesignManager(QObject *parent)
    : QObject(parent)
{
    if (_d) return;
    _d = new DesignManagerPrivate(this);
}

DesignManager* DesignManager::instance()
{
    return _d->parent;
}

DesignManager::~DesignManager()
{
    delete _d;
}

void DesignManager::setSettleWidget(QWidget* widget)
{
    _d->settleWidget = widget;
    if (_d->settleWidget)
        _d->settleWidget->setLayout(&_d->layout);
}

void DesignManager::showWidget()
{
    _d->designerView.show();
}

void DesignManager::hideWidget()
{
    _d->designerView.hide();
}

#include "designmanager.moc"
