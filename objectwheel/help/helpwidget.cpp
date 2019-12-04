#include <helpwidget.h>
#include <lineedit.h>
#include <utilityfunctions.h>
#include <textbrowserhelpviewer.h>
#include <helpmanager.h>
#include <paintutils.h>
#include <interfacesettings.h>
#include <generalsettings.h>

#include <QHelpContentWidget>
#include <QHelpIndexWidget>
#include <QHelpEngine>
#include <QVBoxLayout>
#include <QToolBar>
#include <QComboBox>
#include <QToolButton>
#include <QLabel>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMouseEvent>

// TODO: "Find" on help page

HelpWidget::HelpWidget(QWidget* parent) : QSplitter(parent)
  , m_mode(Invalid)
  , m_helpModeComboBox(new QComboBox)
  , m_indexListFilterEdit(new LineEdit)
  , m_titleLabel(new QLabel)
  , m_homeButton(new QToolButton)
  , m_backButton(new QToolButton)
  , m_forthButton(new QToolButton)
  , m_helpViewer(new Help::Internal::TextBrowserHelpViewer)
{
    QHelpEngine* engine = HelpManager::helpEngine();
    engine->indexWidget()->installEventFilter(this);
    engine->contentWidget()->viewport()->installEventFilter(this);

    m_helpModeComboBox->addItem("Contents");
    m_helpModeComboBox->addItem("Index");

    m_homeButton->setFixedSize({20, 20});
    m_backButton->setFixedSize({20, 20});
    m_forthButton->setFixedSize({20, 20});
    m_titleLabel->setFixedHeight(20);
    m_helpModeComboBox->setFixedHeight(20);

    m_homeButton->setCursor(Qt::PointingHandCursor);
    m_backButton->setCursor(Qt::PointingHandCursor);
    m_forthButton->setCursor(Qt::PointingHandCursor);
    m_helpModeComboBox->setCursor(Qt::PointingHandCursor);

    m_homeButton->setToolTip(tr("Go to home page"));
    m_backButton->setToolTip(tr("Go back"));
    m_forthButton->setToolTip(tr("Go forth"));
    m_helpModeComboBox->setToolTip(tr("Change help mode"));

    m_homeButton->setIcon(QIcon(QStringLiteral(":/images/help/home.svg")));
    m_backButton->setIcon(QIcon(QStringLiteral(":/images/help/back.svg")));
    m_forthButton->setIcon(QIcon(QStringLiteral(":/images/help/forth.svg")));

    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_helpModeComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_indexListFilterEdit->setPlaceholderText(tr("Search"));
    m_indexListFilterEdit->setClearButtonEnabled(true);
    m_indexListFilterEdit->addAction(PaintUtils::renderOverlaidPixmap(":/images/search.svg", "#595959",
                                                                      QSize(16, 16), this),
                                     QLineEdit::LeadingPosition);

    m_backButton->setDisabled(true);
    m_forthButton->setDisabled(true);

    auto toolBar = new QToolBar;
    toolBar->layout()->setSpacing(2);
    toolBar->layout()->setContentsMargins(1, 1, 1, 1);
    toolBar->setFixedHeight(22);
    toolBar->addWidget(UtilityFunctions::createSpacingWidget({1, 1}));
    toolBar->addWidget(m_homeButton);
    toolBar->addWidget(m_backButton);
    toolBar->addWidget(m_forthButton);
    toolBar->addSeparator();
    toolBar->addWidget(UtilityFunctions::createSpacingWidget({1, 1}));
    toolBar->addWidget(m_titleLabel);
    toolBar->addWidget(UtilityFunctions::createSpacingWidget({1, 1}));

    auto viewerContainer = new QWidget;
    auto viewerContainerLayout = new QVBoxLayout(viewerContainer);
    viewerContainerLayout->setContentsMargins(0, 0, 0, 0);
    viewerContainerLayout->setSpacing(0);
    viewerContainerLayout->addWidget(toolBar);
    viewerContainerLayout->addWidget(m_helpViewer);

    auto listToolBar = new QToolBar;
    listToolBar->layout()->setSpacing(2);
    listToolBar->layout()->setContentsMargins(1, 1, 1, 1);
    listToolBar->setFixedHeight(22);
    listToolBar->addWidget(m_helpModeComboBox);

    auto listContainerLayout = new QVBoxLayout;
    listContainerLayout->setContentsMargins(4, 4, 4, 4);
    listContainerLayout->setSpacing(4);
    listContainerLayout->addWidget(m_indexListFilterEdit);
    listContainerLayout->addWidget(engine->indexWidget());
    listContainerLayout->addWidget(engine->contentWidget());

    auto menuContainer = new QWidget;
    auto menuContainerLayout = new QVBoxLayout(menuContainer);
    menuContainerLayout->setContentsMargins(0, 0, 0, 0);
    menuContainerLayout->setSpacing(0);
    menuContainerLayout->addWidget(listToolBar);
    menuContainerLayout->addLayout(listContainerLayout);

    setFrameShape(QFrame::NoFrame);
    setChildrenCollapsible(false);
    setOrientation(Qt::Horizontal);
    addWidget(menuContainer);
    addWidget(viewerContainer);
    setSizes(QList<int>() << 120 << 600);

    connect(m_helpModeComboBox, qOverload<int>(&QComboBox::activated),
            this, &HelpWidget::onHelpModeComboBoxActivation);
    connect(m_indexListFilterEdit, &LineEdit::textEdited,
            this, &HelpWidget::onIndexFilterTextEdit);
    connect(m_indexListFilterEdit, &LineEdit::returnPressed,
            engine->indexWidget(), &QHelpIndexWidget::activateCurrentItem);
    connect(engine->contentWidget(), qOverload<const QUrl&>(&QHelpContentWidget::linkActivated),
            this, qOverload<const QUrl&>(&HelpWidget::onUrlChange));
    connect(engine->indexWidget(), qOverload<const QUrl&, const QString&>(&QHelpIndexWidget::linkActivated),
            this, qOverload<const QUrl&, const QString&>(&HelpWidget::onUrlChange));
    connect(engine->indexWidget(), qOverload<const QMap<QString, QUrl>&, const QString&>(&QHelpIndexWidget::linksActivated),
            this, qOverload<const QMap<QString, QUrl>&, const QString&>(&HelpWidget::onUrlChange));
    connect(m_helpViewer, &Help::Internal::TextBrowserHelpViewer::titleChanged,
            this, &HelpWidget::onTitleChange);
    connect(m_helpViewer, &Help::Internal::TextBrowserHelpViewer::backwardAvailable,
            m_backButton, &QToolButton::setEnabled);
    connect(m_helpViewer, &Help::Internal::TextBrowserHelpViewer::forwardAvailable,
            m_forthButton, &QToolButton::setEnabled);
    connect(m_homeButton, &QToolButton::clicked,
            this, &HelpWidget::onHomeButtonClick);
    connect(m_backButton, &QToolButton::clicked,
            m_helpViewer, &Help::Internal::TextBrowserHelpViewer::backward);
    connect(m_forthButton, &QToolButton::clicked,
            m_helpViewer, &Help::Internal::TextBrowserHelpViewer::forward);
    connect(GeneralSettings::instance(), &GeneralSettings::designerStateReset,
            this, [=] {
        InterfaceSettings* settings = GeneralSettings::interfaceSettings();
        settings->begin();
        settings->setValue("HelpWidget.CurrentMode", 0);
        settings->end();
        setMode(ContentList);
    });

    InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    settings->begin();
    const int index = settings->value<int>("HelpWidget.CurrentMode", 0);
    settings->end();
    switch (index) {
    case 0:
        setMode(ContentList);
        break;
    case 1:
        setMode(IndexList);
        break;
    default:
        break;
    }

    m_helpViewer->home();
}

HelpWidget::Mode HelpWidget::mode() const
{
    return m_mode;
}

void HelpWidget::setMode(HelpWidget::Mode mode)
{
    QHelpEngine* engine = HelpManager::helpEngine();

    if (m_mode != mode) {
        m_mode = mode;

        m_indexListFilterEdit->hide();
        engine->indexWidget()->hide();
        engine->contentWidget()->hide();

        switch (mode) {
        case ContentList:
            engine->contentWidget()->show();
            m_helpModeComboBox->setCurrentText(tr("Contents"));
            break;
        case IndexList:
            m_indexListFilterEdit->show();
            engine->indexWidget()->show();
            m_helpModeComboBox->setCurrentText(tr("Index"));
            break;
        default:
            break;
        }
    }
}

QSize HelpWidget::sizeHint() const
{
    return QSize(670, 640);
}

bool HelpWidget::eventFilter(QObject* watched, QEvent* event)
{
    QHelpEngine* engine = HelpManager::helpEngine();
    if (watched == engine->indexWidget() && event->type() == QEvent::KeyPress) {
        auto e = static_cast<QKeyEvent*>(event);
        if (e->key() == Qt::Key_Return)
            engine->indexWidget()->activateCurrentItem();
    } else if (watched == engine->contentWidget()->viewport() && event->type() == QEvent::MouseButtonPress) {
        auto e = static_cast<QMouseEvent*>(event);
        engine->contentWidget()->activated(engine->contentWidget()->indexAt(e->pos()));
    }
    return QWidget::eventFilter(watched, event);
}

void HelpWidget::onHelpModeComboBoxActivation()
{
    if (m_helpModeComboBox->currentText() == tr("Contents"))
        setMode(ContentList);
    else if (m_helpModeComboBox->currentText() == tr("Index"))
        setMode(IndexList);

    if (GeneralSettings::interfaceSettings()->preserveDesignerState) {
        InterfaceSettings* settings = GeneralSettings::interfaceSettings();
        settings->begin();
        settings->setValue("HelpWidget.CurrentMode", m_helpModeComboBox->currentIndex());
        settings->end();
    }
}

void HelpWidget::onHomeButtonClick()
{
    m_helpViewer->stop();
    m_helpViewer->home();
}

void HelpWidget::discharge()
{
    m_helpViewer->stop();
    m_helpViewer->clearHistory();
    m_helpViewer->home();
    m_indexListFilterEdit->clear();
    m_backButton->setDisabled(true);
    m_forthButton->setDisabled(true);
    QHelpEngine* engine = HelpManager::helpEngine();
    engine->indexWidget()->clearSelection();
    engine->contentWidget()->clearSelection();
    engine->indexWidget()->scrollToTop();
    engine->contentWidget()->scrollToTop();
    engine->contentWidget()->collapseAll();
}

void HelpWidget::onTitleChange()
{
    m_titleLabel->setText("<p = \"font-size:12px;\"><b>" + tr("Topic") + ":</b> " + m_helpViewer->title() + "</p>");
}

void HelpWidget::onIndexFilterTextEdit(const QString& filterText)
{
    HelpManager::helpEngine()->indexWidget()->filterIndices(filterText);
}

void HelpWidget::onUrlChange(const QUrl& url)
{
    m_helpViewer->setFocus();
    m_helpViewer->stop();
    m_helpViewer->setSource(url);
}

void HelpWidget::onUrlChange(const QUrl& link, const QString& /*keyword*/)
{
    onUrlChange(link);
}

void HelpWidget::onUrlChange(const QMap<QString, QUrl>& links, const QString& keyword)
{
    bool ok;
    QString item = QInputDialog::getItem(this, tr("Choose Topic"),
                                         tr("Choose a topic for %1:").arg(keyword), links.keys(), 0, false, &ok);
    if (ok && !item.isEmpty())
        onUrlChange(links.value(item));
}
