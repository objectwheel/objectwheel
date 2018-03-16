#include <webenginehelpviewer.h>
#include <utils/qtcassert.h>

#include <QBuffer>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QVBoxLayout>
#include <QWebEngineContextMenuData>
#include <QWebEngineHistory>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineUrlRequestJob>
#include <QHelpEngine>

namespace {
    struct HelpData {
        QUrl resolvedUrl;
        QByteArray data;
        QString mimeType;
    };

    QByteArray loadErrorMessage(const QUrl &url, const QString &errorString);
}

HelpUrlSchemeHandler::HelpUrlSchemeHandler(QHelpEngine* helpEngine, QObject *parent) : QWebEngineUrlSchemeHandler(parent)
  , m_helpEngine(helpEngine)
{
}

void HelpUrlSchemeHandler::requestStarted(QWebEngineUrlRequestJob *job)
{
    const QUrl url = job->requestUrl();
    if (!HelpViewer::isLocalUrl(url)) {
        job->fail(QWebEngineUrlRequestJob::RequestDenied);
        return;
    }
    HelpData data;

    data.resolvedUrl = m_helpEngine->findFile(url);
    if (data.resolvedUrl.isValid()) {
        data.data = m_helpEngine->fileData(data.resolvedUrl);
        data.mimeType = HelpViewer::mimeFromUrl(data.resolvedUrl);
        if (data.mimeType.isEmpty())
            data.mimeType = "application/octet-stream";
    } else {
        data.data = loadErrorMessage(url, QCoreApplication::translate(
                                         "Help", "The page could not be found"));
        data.mimeType = "text/html";
    }

    auto buffer = new QBuffer(job);
    buffer->setData(data.data);
    job->reply(data.mimeType.toUtf8(), buffer);
}

static HelpUrlSchemeHandler *helpUrlSchemeHandler(QHelpEngine* helpEngine, QObject* parent)
{
    static HelpUrlSchemeHandler *schemeHandler = nullptr;
    if (!schemeHandler)
        schemeHandler = new HelpUrlSchemeHandler(helpEngine, parent);
    return schemeHandler;
}

WebEngineHelpViewer::WebEngineHelpViewer(QHelpEngine* helpEngine, QWidget *parent) : HelpViewer(helpEngine, parent)
    , m_widget(new WebView(this))
{
    m_widget->setPage(new WebEngineHelpPage(this));
    auto layout = new QVBoxLayout;
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_widget, 10);

    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::white);
    p.setColor(QPalette::Text, Qt::black);
    setPalette(p);

    connect(m_widget, &QWebEngineView::urlChanged, this, &WebEngineHelpViewer::sourceChanged);
    connect(m_widget, &QWebEngineView::loadStarted, this, &WebEngineHelpViewer::slotLoadStarted);
    connect(m_widget, &QWebEngineView::loadFinished, this, &WebEngineHelpViewer::slotLoadFinished);
    connect(m_widget, &QWebEngineView::titleChanged, this, &WebEngineHelpViewer::titleChanged);
    connect(m_widget->page(), &QWebEnginePage::linkHovered, this, &WebEngineHelpViewer::setToolTip);
    connect(m_widget->pageAction(QWebEnginePage::Back), &QAction::changed, this, [this]() {
        emit backwardAvailable(isBackwardAvailable());
    });
    connect(m_widget->pageAction(QWebEnginePage::Forward), &QAction::changed, this, [this]() {
        emit forwardAvailable(isForwardAvailable());
    });

//    QAction* action = m_widget->pageAction(QWebEnginePage::OpenLinkInNewTab);
//    action->setText(QCoreApplication::translate("HelpViewer",
//                                                Constants::TR_OPEN_LINK_AS_NEW_PAGE));

    QWebEnginePage *viewPage = m_widget->page();
    QTC_ASSERT(viewPage, return);
    QWebEngineProfile *viewProfile = viewPage->profile();
    QTC_ASSERT(viewProfile, return);
    if (!viewProfile->urlSchemeHandler("qthelp"))
        viewProfile->installUrlSchemeHandler("qthelp", helpUrlSchemeHandler(m_helpEngine, this));
}

QFont WebEngineHelpViewer::viewerFont() const
{
    QWebEngineSettings *webSettings = m_widget->settings();
    return QFont(webSettings->fontFamily(QWebEngineSettings::StandardFont),
                 webSettings->fontSize(QWebEngineSettings::DefaultFontSize));
}

void WebEngineHelpViewer::setViewerFont(const QFont &font)
{
    QWebEngineSettings *webSettings = m_widget->settings();
    webSettings->setFontFamily(QWebEngineSettings::StandardFont, font.family());
    webSettings->setFontSize(QWebEngineSettings::DefaultFontSize, font.pointSize());
}

qreal WebEngineHelpViewer::scale() const
{
    return m_widget->zoomFactor();
}

void WebEngineHelpViewer::setScale(qreal scale)
{
    m_widget->setZoomFactor(scale);
}

QString WebEngineHelpViewer::title() const
{
    return m_widget->title();
}

QUrl WebEngineHelpViewer::source() const
{
    return m_widget->url();
}

void WebEngineHelpViewer::setSource(const QUrl &url)
{
    m_widget->setUrl(url);
}

void WebEngineHelpViewer::setHtml(const QString &html)
{
    m_widget->setHtml(html);
}

QString WebEngineHelpViewer::selectedText() const
{
    return m_widget->selectedText();
}

bool WebEngineHelpViewer::isForwardAvailable() const
{
    // m_view->history()->canGoForward()
    return m_widget->pageAction(QWebEnginePage::Forward)->isEnabled();
}

bool WebEngineHelpViewer::isBackwardAvailable() const
{
    return m_widget->pageAction(QWebEnginePage::Back)->isEnabled();
}

void WebEngineHelpViewer::addBackHistoryItems(QMenu *backMenu)
{
    if (QWebEngineHistory *history = m_widget->history()) {
        QList<QWebEngineHistoryItem> items = history->backItems(history->count());
        for (int i = items.count() - 1; i >= 0; --i) {
            QWebEngineHistoryItem item = items.at(i);
            auto action = new QAction(backMenu);
            action->setText(item.title());
            connect(action, &QAction::triggered, this, [this,item]() {
                if (QWebEngineHistory *history = m_widget->history())
                    history->goToItem(item);
            });
            backMenu->addAction(action);
        }
    }
}

void WebEngineHelpViewer::addForwardHistoryItems(QMenu *forwardMenu)
{
    if (QWebEngineHistory *history = m_widget->history()) {
        QList<QWebEngineHistoryItem> items = history->forwardItems(history->count());
        for (int i = 0; i < items.count(); ++i) {
            QWebEngineHistoryItem item = items.at(i);
            auto action = new QAction(forwardMenu);
            action->setText(item.title());
            connect(action, &QAction::triggered, this, [this,item]() {
                if (QWebEngineHistory *history = m_widget->history())
                    history->goToItem(item);
            });
            forwardMenu->addAction(action);
        }
    }
}

WebEngineHelpPage *WebEngineHelpViewer::page() const
{
    return static_cast<WebEngineHelpPage *>(m_widget->page());
}

void WebEngineHelpViewer::scaleUp()
{
    m_widget->setZoomFactor(m_widget->zoomFactor() + 0.1);
}

void WebEngineHelpViewer::scaleDown()
{
    m_widget->setZoomFactor(qMax(qreal(0.1), m_widget->zoomFactor() - qreal(0.1)));
}

void WebEngineHelpViewer::resetScale()
{
    m_widget->setZoomFactor(1.0);
}

void WebEngineHelpViewer::copy()
{
    m_widget->triggerPageAction(QWebEnginePage::Copy);
}

void WebEngineHelpViewer::stop()
{
    m_widget->triggerPageAction(QWebEnginePage::Stop);
}

void WebEngineHelpViewer::forward()
{
    m_widget->triggerPageAction(QWebEnginePage::Forward);
}

void WebEngineHelpViewer::backward()
{
    m_widget->triggerPageAction(QWebEnginePage::Back);
}

void WebEngineHelpViewer::print(QPrinter *printer)
{
    Q_UNUSED(printer)
}

WebEngineHelpPage::WebEngineHelpPage(QObject *parent)
    : QWebEnginePage(parent)
{
}

WebView::WebView(WebEngineHelpViewer *viewer)
    : QWebEngineView(viewer),
      m_viewer(viewer)
{
}

void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = page()->createStandardContextMenu();
    // insert Open as New Page etc if OpenLinkInThisWindow is also there
    const QList<QAction*> actions = menu->actions();
    auto it = std::find(actions.cbegin(), actions.cend(),
                        page()->action(QWebEnginePage::OpenLinkInThisWindow));
    if (it != actions.cend()) {
        // insert after
        ++it;
        QAction *before = (it == actions.cend() ? 0 : *it);

        if (m_viewer->isActionVisible(HelpViewer::Action::NewPage)) {
            QAction *openLinkInNewTab = page()->action(QWebEnginePage::OpenLinkInNewTab);
            menu->insertAction(before, openLinkInNewTab);
        }
    }

    connect(menu, &QMenu::aboutToHide, menu, &QObject::deleteLater);
    menu->popup(event->globalPos());
}

namespace {
    QByteArray loadErrorMessage(const QUrl &url, const QString &errorString)
    {
        const char g_htmlPage[] =
            "<html>"
            "<head>"
            "<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">"
            "<title>%1</title>"
            "<style>"
            "body {padding: 3em 0em; background: #eeeeee;}"
            "hr {color: lightgray; width: 100%;}"
            "img {float: left; opacity: .8;}"
            "#box {background: white; border: 1px solid lightgray; width: 600px; padding: 60px; margin: auto;}"
            "h1 {font-size: 130%; font-weight: bold; border-bottom: 1px solid lightgray; margin-left: 48px;}"
            "h2 {font-size: 100%; font-weight: normal; border-bottom: 1px solid lightgray; margin-left: 48px;}"
            "p {font-size: 90%; margin-left: 48px;}"
            "</style>"
            "</head>"
            "<body>"
            "<div id=\"box\">"
            "<img "
                "src=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACA"
                "AAAAgCAYAAABzenr0AAAABHNCSVQICAgIfAhkiAAAAAlwSFlzAAAOxAAADsQBlSsOGwAABnxJREFUWIXtlltsHGcVx3/fzO7MXuy92X"
                "ux17u+Jb61bEMiCk0INCjw0AckEIaHcH2oH5CSoChQAq0s8RJD5SYbiFOrUlOqEOChlSioREIlqkJoKwFKSoNQktpxUpqNE3vXu/ZeZ"
                "me+j4e1gwKRYruK+sKRPmk0M+ec3/mf78w38H/7kE2sx2lyctLdGov9UNWd6nxh/tTIyMi59QJo63Fyu8V2Xbj3BkPxgyB2jY6OrisO"
                "gGutDtlstsMwA8eDQT2k6zeIxZJ7pHTOAyfWA7Bmcp/Ps8frjadrpVdxl/fh92uGxxv5zvj4c5H7DnDs2JGHg8HEtwVzpFtPkOrNIRa"
                "OEo13b/H7nb33FWB4eFj3+0Pf9/nCfo/9SwYfyZPcYBFtfR0PF4i0pB8fGxt74L4B7NixYzgYbP+8pr1Hf8/vbt/PbC8i55+nra2rLR"
                "Rq2ccaJ2tVABMTB8OBQORHkUhCN8on6NlSgyqNBcRjf8VUfybVObTr2Z89+5m1AKxqCoQIPR6Ndm6U9hk6U68xObGFy5fDCF3i8+p87"
                "QtvUpw6SrjjRbMQjjyRzWb/tHfv3tpqYt9TgSNHjgwkEqn9rVETd+UknQ/UuPDPzSwsbiW/8DDTMw+RuxGhK30ZNX+Szp6hnVKyazXJ"
                "7wkwOjqqBQKBfX39mahV/iPtqbdQSsfrKaNpJQRFFPNoCJIb6tTnXqG3s1WkuzbuHx8/lvzAAJFIZHt7csNXS6VrhGSWzqE6utCQdpn"
                "S4hILxQUKhTl0HLCb6eud5tLZJ9m27dODTU3a7g8EkM1mzZaW6NOZTMZbn/85HT03oBrGrrqxnUUKhQL5fIFSsQhOHWqSlrBEVH5PMf"
                "cWfYObvnX06NHMugF0Xf96Kt2/eebKadqDv6GpyQt1ExTYtSXm5uYpFheQTg0NBywLaet0x3P86+2nyTz4kZjfH9g/PDysrxlgfHw8m"
                "WhLPdnf36OX33+enqEyWH6wNXB0apUSxeIijqPweHRM3Qa7hqxZtEQcguo1Lr05wcDQli9u3br1c2sGCATCBwcGtqSnL75MV/Qs1P1I"
                "S0DVwcm7mL+VY3p6itnZG1TKizjlReyiRb1Sp1aGnpjF/KVjdHUl/G3J9A8mJyeDqwY4fPjwg9FY22MuvYQ9e5Ku7iK1fJFK/jrVfA6"
                "rmKeYv0m1MksudxPHqSJrNtYiOEvglIA6JIxrXHz9x/T2bfqktOWXVgUwMjLiDgTChwcGMi1X//4Mgx2nWcpZVAtlrJLEXgLdAc/y5y"
                "scaaEt3oqhg6oDFuCAbUNn3KJ85TgsTRFrT313fHz8rmN5B0Amk3ksGks9emX6DeL6r/C5JHUblA1IUA64dAg1A7jw+lswDROhGs+Ro"
                "GTjfSWhOzDH7Pmf0tbR1+/1evfcDeD2wXHo0KFQazTxRnf30MDSlVE+2vEKblOiHGAlgQJNwcwMXL0OHi8EfZAMgccA6TQS44CU4BZw"
                "4ZpBpesgNf/mhZl339m5e/fuv9xVAZ+v6alYYsPAws3TdHhfxTBlQ1ansVQdlAVaHWwH3s3B2XcMbuUh6AVpLbfBBsdpqGXVob3ZoTr"
                "za0LB1mBTU/P3/lsBfbn6rnBL4pDHsJvdxeP0xqYQQt2WdQVCo9GCiZfgqefc/ONGBunp5KHke/iNRtVyRa1lfX0eRaV4k/myl6bkIx"
                "s//rFN50+dOnXxDgWam4PPBEPxdnvxNCn/GTxeHU0YaJobTdMQukDXwK2D0GE6B+AmnQ5T1zspWwZuE4ThQne70U0D3TRwmW6EYdARd"
                "9BmX8aj2UZzKPrE2NjY7bF0TUxkPxEIhD/rVC8T4W/0DaawLAO3oxrlKIVSEqEa16ZLsv+bkoow8IYNPjV4nWRHEpfPxFMXKARCY3nj"
                "NDZZc0xScIpMT/2C1uSubeVS4RvAEQDxwgsv/iGeSO9Uxd8Ss15CKeM/0qsVLRsB1XJQF1C2oFJx8HkFLl1Hoa/kBHHnb5EANN2mUI0"
                "i0we4tehcnZme2XHgwL4pl9BELBJpwhv/MoKvAAKBhtAEQghMj4nhNjE9Xlwu13J1opFAgFpOKh0bq26Dgmp5iZpVQ0qJUgolGyomhI"
                "atNMRcvj176Ce9wJQrd/39M+WlpY5are66PRQaaKIhpSY0BHqjKpfAtVKbaEAoANXAsFEoe7ltOEipaHROoZRCAEIooZS8fO7cuUsr6"
                "gDc89i8D/b2h5Dzf+3fzO2jy1yqBcAAAAAASUVORK5CYII=\""
                "width=\"32\" height=\"32\"/>"
            "<h1>%2</h1>"
            "<h2>%3</h2>"
            "%4"
            "</div>"
            "</body>"
            "</html>";

        // some of the values we will replace %1...6 inside the former html
        const QString g_percent1 = QCoreApplication::translate("Help", "Error loading page");
        // percent2 will be the error details
        // percent3 will be the url of the page we got the error from
        const QString g_percent4 = QCoreApplication::translate("Help", "<p>Check that you have the corresponding "
            "documentation set installed.</p>");

        return QString::fromLatin1(g_htmlPage).arg(g_percent1, errorString,
                    QCoreApplication::translate("Help", "Error loading: %1").arg(url.toString()),
                    g_percent4).toUtf8();
    }
}