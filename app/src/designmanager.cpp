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

    public:
        DesignManager* parent;
        QWidget* settleWidget = nullptr;
        QVBoxLayout layout;
        DesignerScene designerScene;
        DesignerView designerView;
        QmlPreviewer qmlPreviewer;
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
