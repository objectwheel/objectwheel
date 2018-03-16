#include <helpwidget.h>
#include <fit.h>
#include <webenginehelpviewer.h>

#include <QtWidgets>
#include <QtHelp>
#include <QtWebEngineWidgets>

namespace {
    bool isValidUrl(const QString &link);
}

HelpWidget::HelpWidget(QWidget *parent) : QWidget(parent)
  , m_helpEngine(new QHelpEngine("/Users/omergoktas/Desktop/docs/docs.qhc", this))
  , m_layout(new QGridLayout(this))
  , m_indexTypeCombo(new QComboBox)
  , m_helpViewer(new WebEngineHelpViewer(m_helpEngine))
{
    //! [1]
    m_helpEngine->setAutoSaveFilter(false);
    m_helpEngine->setupData();
    m_helpEngine->indexWidget()->setFixedWidth(fit::fx(150));
    m_helpEngine->contentWidget()->setFixedWidth(fit::fx(150));
    m_indexTypeCombo->setFixedWidth(fit::fx(150));
    //! [1]

    //! [2]
    m_indexTypeCombo->addItem("Index");
    m_indexTypeCombo->addItem("Contents");
    //! [2]

    //! [3]
    connect(m_indexTypeCombo, &QComboBox::currentTextChanged, [=] {
        m_layout->removeWidget(m_helpEngine->indexWidget());
        m_layout->removeWidget(m_helpEngine->contentWidget());

        if (m_indexTypeCombo->currentIndex() == 0)
            m_layout->addWidget(m_helpEngine->indexWidget(), 1, 0, 1, 1);
        else
            m_layout->addWidget(m_helpEngine->contentWidget(), 1, 0, 1, 1);
    });
    //! [3]

    //! [4]
    connect(m_helpEngine->contentWidget(), &QHelpContentWidget::activated, [=] {
        auto index = m_helpEngine->contentWidget()->currentIndex();
        showHelpForKeyword(index.data(Qt::DisplayRole).toString());
    });
    //! [4]

    //! [5]
    connect(m_helpEngine->indexWidget(), &QHelpIndexWidget::activated, [=] {
        auto index = m_helpEngine->indexWidget()->currentIndex();
        showHelpForKeyword(index.data(Qt::DisplayRole).toString());
    });
    //! [5]

    //! [6]
    m_layout->addWidget(m_indexTypeCombo, 0, 0, 1, 1);
    m_layout->addWidget(m_helpEngine->indexWidget(), 1, 0, 1, 1);
    m_layout->addWidget(m_helpViewer, 0, 1, 2, 1);
    //! [6]
}

void HelpWidget::showHelpForKeyword(const QString &id)
{
    QMap<QString, QUrl> links = m_helpEngine->linksForIdentifier(id);
    // Maybe the id is already an URL
    if (links.isEmpty() && isValidUrl(id))
        links.insert(id, id);

    QUrl source;

    if (!links.isEmpty())
        source = links.first();

    if (!source.isValid()) {
        // No link found or no context object
        m_helpViewer->setSource(QUrl());
        m_helpViewer->setHtml(tr("<html><head><title>No Documentation</title>"
            "</head><body><br/><center>"
            "<font color=\"%1\"><b>%2</b></font><br/>"
            "<font color=\"%3\">No documentation available.</font>"
            "</center></body></html>")
            .arg("#202427")
            .arg(id)
            .arg("#202427"));
    } else {
        m_helpViewer->setFocus();
        m_helpViewer->stop();
        m_helpViewer->setSource(source); // triggers loadFinished which triggers id highlighting
        qDebug() << source;
    }
}

QSize HelpWidget::sizeHint() const
{
    return fit::fx(QSizeF(680, 680)).toSize();
}

void HelpWidget::reset()
{
    //TODO
}

namespace {
    /*!
     * Checks if the string does contain a scheme, and if that scheme is a "sensible" scheme for
     * opening in a internal or external browser (qthelp, about, file, http, https).
     * This is necessary to avoid trying to open e.g. "Foo::bar" in a external browser.
     */
    bool isValidUrl(const QString &link)
    {
        QUrl url(link);
        if (!url.isValid())
            return false;
        const QString scheme = url.scheme();
        return (scheme == "qthelp"
                || scheme == "about"
                || scheme == "file"
                || scheme == "http"
                || scheme == "https");
    }
}