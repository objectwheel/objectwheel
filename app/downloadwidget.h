#ifndef DOWNLOADWIDGET_H
#define DOWNLOADWIDGET_H

#include <QQuickWidget>
#include <global.h>

class DownloadWidgetPrivate;

class DownloadWidget : public QQuickWidget
{
    Q_OBJECT
public:
    explicit DownloadWidget(QWidget *parent = 0);
    ~DownloadWidget();
    static DownloadWidget* instance();

private:
    static DownloadWidgetPrivate* _d;
    void resizeEvent(QResizeEvent *event) override;

signals:
    void done();
    void resized() const;

public slots:
    void download(OTargets::Targets);
    void handleBtnOkClicked();
    void handleBtnCancelClicked();
};

#endif // DOWNLOADWIDGET_H
