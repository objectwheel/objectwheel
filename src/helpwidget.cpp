#include <helpwidget.h>
#include <fit.h>
#include <webenginehelpviewer.h>
#include <focuslesslineedit.h>
#include <css.h>

#include <QtWidgets>
#include <QtHelp>
#include <QtWebEngineWidgets>

// TODO: "Find" on help page

namespace {
    QHelpContentWidget* contentWidget;
    QHelpIndexWidget* indexWidget;
}

HelpWidget::HelpWidget(QWidget *parent) : QWidget(parent)
  , m_helpEngine(new QHelpEngine("./docs/docs.qhc", this))
  , m_layout(new QVBoxLayout(this))
  , m_toolbar(new QToolBar)
  , m_typeCombo(new QComboBox)
  , m_homeButton(new QToolButton)
  , m_backButton(new QToolButton)
  , m_forthButton(new QToolButton)
  , m_titleLabel(new QLabel)
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
    m_layout->addWidget(m_toolbar);
    m_layout->addWidget(m_splitter);

    m_toolbar->setStyleSheet(CSS::DesignerToolbar);
    m_toolbar->setIconSize(QSize(fit::fx(14), fit::fx(14)));
    m_toolbar->setFixedHeight(fit::fx(21));
    m_toolbar->addWidget(m_typeCombo);
    m_toolbar->addSeparator();
    m_toolbar->addWidget(m_homeButton);
    m_toolbar->addWidget(m_backButton);
    m_toolbar->addWidget(m_forthButton);
    m_toolbar->addSeparator();
    m_toolbar->addWidget(m_titleLabel);

    QFont f;
    f.setPixelSize(f.pixelSize() - 1);
    m_titleLabel->setFont(f);
    m_titleLabel->setTextFormat(Qt::RichText);

    m_typeCombo->setFixedWidth(fit::fx(280));
    m_typeCombo->addItem("Index");
    m_typeCombo->addItem("Contents");

    m_homeButton->setCursor(Qt::PointingHandCursor);
    m_backButton->setCursor(Qt::PointingHandCursor);
    m_forthButton->setCursor(Qt::PointingHandCursor);

    m_homeButton->setToolTip(tr("Go Home"));
    m_backButton->setToolTip(tr("Go Back"));
    m_forthButton->setToolTip(tr("Go Forth"));

    m_homeButton->setIcon(QIcon(":/resources/images/home.png"));
    m_backButton->setIcon(QIcon(":/resources/images/undo.png"));
    m_forthButton->setIcon(QIcon(":/resources/images/redo.png"));

    m_splitter->setHandleWidth(0);
    m_splitter->setOrientation(Qt::Horizontal);
    m_splitter->addWidget(m_indexWidget);
    m_splitter->addWidget(m_helpViewer);
    m_splitter->setSizes(QList<int>() << fit::fx(140) << fit::fx(600));

    contentWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
    indexWidget->setAttribute(Qt::WA_MacShowFocusRect, false);

    m_contentsLayout->setSpacing(fit::fx(5));
    m_contentsLayout->setContentsMargins(fit::fx(5), fit::fx(5), fit::fx(5), fit::fx(5));
    m_contentsLayout->addWidget(contentWidget);
    m_indexFilterEdit->setClearButtonEnabled(true);

    m_indexLayout->setSpacing(fit::fx(5));
    m_indexLayout->setContentsMargins(fit::fx(5), fit::fx(5), fit::fx(5), fit::fx(5));
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
}

QSize HelpWidget::sizeHint() const
{
    return fit::fx(QSizeF(680, 680)).toSize();
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

void HelpWidget::reset()
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