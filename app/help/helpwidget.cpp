#include <helpwidget.h>
#include <focuslesslineedit.h>
#include <transparentstyle.h>
#include <utilsicons.h>
#include <utilityfunctions.h>
#include <textbrowserhelpviewer.h>
#include <helpmanager.h>

#include <QHelpContentWidget>
#include <QHelpIndexWidget>
#include <QHelpEngine>
#include <QVBoxLayout>
#include <QToolBar>
#include <QComboBox>
#include <QToolButton>
#include <QLabel>
#include <QSplitter>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QClipboard>

// TODO: "Find" on help page
// TODO: Copy/paste keyboard shortcuts

HelpWidget::HelpWidget(QWidget *parent) : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_toolBar(new QToolBar)
  , m_typeCombo(new QComboBox)
  , m_homeButton(new QToolButton)
  , m_backButton(new QToolButton)
  , m_forthButton(new QToolButton)
  , m_titleLabel(new QLabel)
  , m_copyAction(new QAction(this))
  , m_splitter(new QSplitter)
  , m_helpViewer(new Help::Internal::TextBrowserHelpViewer)
  , m_contentsWidget(new QWidget)
  , m_contentsLayout(new QVBoxLayout(m_contentsWidget))
  , m_indexWidget(new QWidget)
  , m_indexLayout(new QVBoxLayout(m_indexWidget))
  , m_indexFilterEdit(new FocuslessLineEdit)
{
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_toolBar);
    m_layout->addWidget(m_splitter);

    TransparentStyle::attach(m_toolBar);

    m_homeButton->setFixedHeight(20);
    m_backButton->setFixedHeight(20);
    m_forthButton->setFixedHeight(20);
    m_titleLabel->setFixedHeight(20);
    m_typeCombo->setFixedHeight(20);

    m_toolBar->setFixedHeight(24);
    m_toolBar->addWidget(UtilityFunctions::createSpacingWidget({2, 2}));
    m_toolBar->addWidget(m_typeCombo);
    m_toolBar->addWidget(UtilityFunctions::createSpacingWidget({1, 1}));
    m_toolBar->addSeparator();
    m_toolBar->addWidget(UtilityFunctions::createSpacingWidget({1, 1}));
    m_toolBar->addWidget(m_homeButton);
    m_toolBar->addWidget(m_backButton);
    m_toolBar->addWidget(m_forthButton);
    m_toolBar->addWidget(UtilityFunctions::createSpacingWidget({1, 1}));
    m_toolBar->addSeparator();
    m_toolBar->addWidget(UtilityFunctions::createSpacingWidget({10, 10}));
    m_toolBar->addWidget(m_titleLabel);

    UtilityFunctions::adjustFontPixelSize(m_titleLabel, -1);
    m_titleLabel->setTextFormat(Qt::RichText);

    m_typeCombo->setFixedWidth(220);
    m_typeCombo->addItem("Index");
    m_typeCombo->addItem("Contents");

    m_typeCombo->setCursor(Qt::PointingHandCursor);
    m_homeButton->setCursor(Qt::PointingHandCursor);
    m_backButton->setCursor(Qt::PointingHandCursor);
    m_forthButton->setCursor(Qt::PointingHandCursor);

    m_homeButton->setToolTip(tr("Go Home"));
    m_backButton->setToolTip(tr("Go Back"));
    m_forthButton->setToolTip(tr("Go Forth"));

    m_homeButton->setIcon(Utils::Icons::HOME_TOOLBAR.icon());
    m_backButton->setIcon(Utils::Icons::PREV_TOOLBAR.icon());
    m_forthButton->setIcon(Utils::Icons::NEXT_TOOLBAR.icon());

    m_splitter->setHandleWidth(0);
    m_splitter->setChildrenCollapsible(false);
    m_splitter->setOrientation(Qt::Horizontal);
    m_splitter->addWidget(m_indexWidget);
    m_splitter->addWidget(m_helpViewer);
    m_splitter->setSizes(QList<int>() << 140 << 600);

    QHelpEngine* engine = HelpManager::helpEngine();
    engine->contentWidget()->setAttribute(Qt::WA_MacShowFocusRect, false);
    engine->indexWidget()->setAttribute(Qt::WA_MacShowFocusRect, false);

    m_contentsLayout->setSpacing(5);
    m_contentsLayout->setContentsMargins(5, 5, 5, 5);
    m_contentsLayout->addWidget(engine->contentWidget());
    m_indexFilterEdit->setClearButtonEnabled(true);

    m_indexLayout->setSpacing(5);
    m_indexLayout->setContentsMargins(5, 5, 5, 5);
    m_indexLayout->addWidget(m_indexFilterEdit);
    m_indexLayout->addWidget(engine->indexWidget());

    engine->indexWidget()->installEventFilter(this);
    engine->contentWidget()->viewport()->installEventFilter(this);

    connect(m_typeCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &HelpWidget::onTypeChange);
    connect(m_indexFilterEdit, &FocuslessLineEdit::textChanged,
            this, &HelpWidget::onIndexFilterTextChange);
    connect(m_indexFilterEdit, &FocuslessLineEdit::returnPressed,
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

    m_copyAction->setText("Copy selected");
    m_copyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_copyAction->setShortcut(QKeySequence::Copy);
//    m_helpViewer->webView()->addAction(m_copyAction);
//    connect(m_copyAction, &QAction::triggered, this, [=] {
//        QGuiApplication::clipboard()->setText(m_helpViewer->selectedText());
//    });
}

QSize HelpWidget::sizeHint() const
{
    return QSize(680, 680);
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

void HelpWidget::onHomeButtonClick()
{
    m_helpViewer->stop();
    m_helpViewer->home();
}

void HelpWidget::discharge()
{
    QHelpEngine* engine = HelpManager::helpEngine();
    m_helpViewer->stop();
    m_helpViewer->clearHistory();
    m_helpViewer->home();
    m_typeCombo->setCurrentIndex(0);
    m_indexFilterEdit->clear();
    engine->indexWidget()->clearSelection();
    engine->contentWidget()->clearSelection();
    engine->indexWidget()->scrollToTop();
    engine->contentWidget()->collapseAll();
    m_backButton->setDisabled(true);
    m_forthButton->setDisabled(true);
}

void HelpWidget::onTypeChange()
{
    if (m_typeCombo->currentIndex() == 0)
        m_splitter->replaceWidget(0, m_indexWidget);
    else
        m_splitter->replaceWidget(0, m_contentsWidget);
}

void HelpWidget::onTitleChange()
{
    m_titleLabel->setText(tr("<b>Topic: </b>") + m_helpViewer->title());
}

void HelpWidget::onIndexFilterTextChange(const QString& filterText)
{
    QHelpEngine* engine = HelpManager::helpEngine();
    engine->indexWidget()->filterIndices(filterText);
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
