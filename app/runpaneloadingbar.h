#ifndef RUNPANELOADINGBAR_H
#define RUNPANELOADINGBAR_H

#include <QWidget>

class RunPaneLoadingBar : public QWidget
{
    Q_OBJECT

public:
    explicit RunPaneLoadingBar(QWidget *parent = nullptr);
    void setText(const QString& text);

public slots:
    void busy(int progress, const QString& text);
    void done(const QString& text);
    void error(const QString& text);

private slots:
    void onEndingTimeout();
    void onFaderTimeout();

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    QSize recomputeMinimumSizeHint() const;

private:
    int m_progress;
    QString m_text;
    QTimer* m_timerEnding,* m_timerFader;
};

#endif // RUNPANELOADINGBAR_H