#ifndef HELPVIEWER_H
#define HELPVIEWER_H

#include <QFont>
#include <QMenu>
#include <QString>
#include <QUrl>
#include <QWidget>

class QHelpEngine;

class HelpViewer : public QWidget
{
    Q_OBJECT

public:
    enum class Action {
        NewPage = 0x01,
        ExternalWindow = 0x02
    };
    Q_DECLARE_FLAGS(Actions, Action)

    explicit HelpViewer(QWidget *parent = 0);
    ~HelpViewer() { }

    virtual QFont viewerFont() const = 0;
    virtual void setViewerFont(const QFont &font) = 0;

    virtual qreal scale() const = 0;
    virtual void setScale(qreal scale) = 0;

    virtual QString title() const = 0;

    virtual QUrl source() const = 0;
    // metacall in HelpPlugin::updateSideBarSource
    Q_INVOKABLE virtual void setSource(const QUrl &url) = 0;
    virtual void highlightId(const QString &id) { Q_UNUSED(id) }

    virtual void setHtml(const QString &html) = 0;

    virtual QString selectedText() const = 0;
    virtual bool isForwardAvailable() const = 0;
    virtual bool isBackwardAvailable() const = 0;
    virtual void addBackHistoryItems(QMenu *backMenu) = 0;
    virtual void addForwardHistoryItems(QMenu *forwardMenu) = 0;
    void setActionVisible(Action action, bool visible);
    bool isActionVisible(Action action);

    bool handleForwardBackwardMouseButtons(QMouseEvent *e);

    static bool isLocalUrl(const QUrl &url);
    static bool canOpenPage(const QString &url);
    static QString mimeFromUrl(const QUrl &url);

    void home();

    virtual void scaleUp() = 0;
    virtual void scaleDown() = 0;
    virtual void resetScale() = 0;
    virtual void copy() = 0;
    virtual void stop() = 0;
    virtual void forward() = 0;
    virtual void backward() = 0;

signals:
    void sourceChanged(const QUrl &);
    void titleChanged();
    void printRequested();
    void forwardAvailable(bool);
    void backwardAvailable(bool);
    void loadFinished();
    void newPageRequested(const QUrl &url);
    void externalPageRequested(const QUrl &url);

protected:
    void slotLoadStarted();
    void slotLoadFinished();

    Actions m_visibleActions = 0;
};

#endif // HELPVIEWER_H