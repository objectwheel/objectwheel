#ifndef WEBENGINEHELPVIEWER_H
#define WEBENGINEHELPVIEWER_H

#include <helpviewer.h>
#include <QWebEngineUrlSchemeHandler>
#include <QWebEngineView>

class WebEngineHelpViewer;

class HelpUrlSchemeHandler : public QWebEngineUrlSchemeHandler
{
    public:
        explicit HelpUrlSchemeHandler(QHelpEngine* helpEngine, QObject *parent = 0);
        void requestStarted(QWebEngineUrlRequestJob *job) override;

    private:
        QHelpEngine* m_helpEngine;
};

class WebEngineHelpPage : public QWebEnginePage
{
    public:
        explicit WebEngineHelpPage(QObject *parent = 0);
};

class WebView : public QWebEngineView
{
    public:
        explicit WebView(WebEngineHelpViewer *viewer);

    protected:
        void contextMenuEvent(QContextMenuEvent *event) override;

    private:
        WebEngineHelpViewer *m_viewer;
};

class WebEngineHelpViewer : public HelpViewer
{
        Q_OBJECT

    public:
        explicit WebEngineHelpViewer(QHelpEngine* helpEngine, QWidget *parent = 0);

        QFont viewerFont() const override;
        void setViewerFont(const QFont &font) override;
        qreal scale() const override;
        void setScale(qreal scale) override;
        QString title() const override;
        QUrl source() const override;
        void setSource(const QUrl &url) override;
        void setHtml(const QString &html) override;
        QString selectedText() const override;
        bool isForwardAvailable() const override;
        bool isBackwardAvailable() const override;
        void addBackHistoryItems(QMenu *backMenu) override;
        void addForwardHistoryItems(QMenu *forwardMenu) override;

        WebEngineHelpPage *page() const;

    public slots:
        void scaleUp() override;
        void scaleDown() override;
        void resetScale() override;
        void copy() override;
        void stop() override;
        void forward() override;
        void backward() override;
        void print(QPrinter *printer) override;

    private:
        WebView *m_widget;
};

#endif // WEBENGINEHELPVIEWER_H