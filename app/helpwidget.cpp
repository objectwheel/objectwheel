#include <helpwidget.h>
#include <webenginehelpviewer.h>
#include <focuslesslineedit.h>
#include <transparentstyle.h>
#include <utilsicons.h>
#include <appfontsettings.h>
#include <utilityfunctions.h>

#include <QtWidgets>
#include <QtHelp>
#include <QtWebEngineWidgets>

// TODO: "Find" on help page
// TODO: Copy/paste keyboard shortcuts

namespace {
QHelpContentWidget* contentWidget;
QHelpIndexWidget* indexWidget;
}

HelpWidget::HelpWidget(QWidget *parent) : QWidget(parent)
  , m_helpEngine(new QHelpEngine(qApp->applicationDirPath() + "/docs/docs.qhc", this))
  , m_layout(new QVBoxLayout(this))
  , m_toolBar(new QToolBar)
  , m_typeCombo(new QComboBox)
  , m_homeButton(new QToolButton)
  , m_backButton(new QToolButton)
  , m_forthButton(new QToolButton)
  , m_titleLabel(new QLabel)
  , m_copyAction(new QAction(this))
  , m_splitter(new QSplitter)
  , m_helpViewer(new WebEngineHelpViewer(m_helpEngine))
  , m_contentsWidget(new QWidget)
  , m_contentsLayout(new QVBoxLayout(m_contentsWidget))
  , m_indexWidget(new QWidget)
  , m_indexLayout(new QVBoxLayout(m_indexWidget))
  , m_indexFilterEdit(new FocuslessLineEdit)
{
    m_helpEngine->setAutoSaveFilter(false);

    contentWidget = m_helpEngine->contentWidget();
    indexWidget = m_helpEngine->indexWidget();

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
    m_splitter->setOrientation(Qt::Horizontal);
    m_splitter->addWidget(m_indexWidget);
    m_splitter->addWidget(m_helpViewer);
    m_splitter->setSizes(QList<int>() << 140 << 600);

    contentWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
    indexWidget->setAttribute(Qt::WA_MacShowFocusRect, false);

    m_contentsLayout->setSpacing(5);
    m_contentsLayout->setContentsMargins(5, 5, 5, 5);
    m_contentsLayout->addWidget(contentWidget);
    m_indexFilterEdit->setClearButtonEnabled(true);

    m_indexLayout->setSpacing(5);
    m_indexLayout->setContentsMargins(5, 5, 5, 5);
    m_indexLayout->addWidget(m_indexFilterEdit);
    m_indexLayout->addWidget(indexWidget);

    indexWidget->installEventFilter(this);
    contentWidget->viewport()->installEventFilter(this);

    connect(m_typeCombo, SIGNAL(currentIndexChanged(int)), SLOT(onTypeChange()));
    connect(m_indexFilterEdit, SIGNAL(textChanged(QString)), SLOT(onIndexFilterTextChange(QString)));
    connect(m_indexFilterEdit, SIGNAL(returnPressed()), indexWidget, SLOT(activateCurrentItem()));
    connect(contentWidget, SIGNAL(linkActivated(QUrl)), SLOT(onUrlChange(QUrl)));
    connect(indexWidget, SIGNAL(linkActivated(QUrl,QString)), SLOT(onUrlChange(QUrl,QString)));
    connect(indexWidget, SIGNAL(linksActivated(QMap<QString,QUrl>,QString)), SLOT(onUrlChange(QMap<QString,QUrl>,QString)));
    connect(m_helpViewer, SIGNAL(titleChanged()), SLOT(onTitleChange()));
    connect(m_helpViewer, SIGNAL(backwardAvailable(bool)), m_backButton, SLOT(setEnabled(bool)));
    connect(m_helpViewer, SIGNAL(forwardAvailable(bool)), m_forthButton, SLOT(setEnabled(bool)));
    connect(m_homeButton, SIGNAL(clicked(bool)), SLOT(onHomeButtonClick()));
    connect(m_backButton, SIGNAL(clicked(bool)), m_helpViewer, SLOT(backward()));
    connect(m_forthButton, SIGNAL(clicked(bool)), m_helpViewer, SLOT(forward()));

    m_copyAction->setText("Copy selected");
    m_copyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_copyAction->setShortcut(QKeySequence::Copy);
//    m_helpViewer->webView()->addAction(m_copyAction);
    connect(m_copyAction, &QAction::triggered, this, [=] {
        QGuiApplication::clipboard()->setText(m_helpViewer->selectedText());
    });
}

QSize HelpWidget::sizeHint() const
{
    return QSize(680, 680);
}

bool HelpWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == indexWidget && event->type() == QEvent::KeyPress) {
        auto e = static_cast<QKeyEvent*>(event);
        if (e->key() == Qt::Key_Return)
            indexWidget->activateCurrentItem();
    } else if (watched == contentWidget->viewport() && event->type() == QEvent::MouseButtonPress) {
        auto e = static_cast<QMouseEvent*>(event);
        contentWidget->activated(contentWidget->indexAt(e->pos()));
    }
    return QWidget::eventFilter(watched, event);
}

void HelpWidget::onHomeButtonClick()
{
    m_helpViewer->stop();
    m_helpViewer->home();
}

void HelpWidget::sweep()
{
    m_helpViewer->stop();
    m_helpViewer->page()->history()->clear();
    m_helpViewer->home();
    m_typeCombo->setCurrentIndex(0);
    m_indexFilterEdit->clear();
    indexWidget->clearSelection();
    contentWidget->clearSelection();
    indexWidget->scrollToTop();
    contentWidget->collapseAll();
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
    indexWidget->filterIndices(filterText);
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
