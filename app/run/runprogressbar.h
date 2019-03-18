#ifndef RUNPROGRESSBAR_H
#define RUNPROGRESSBAR_H

#include <QWidget>
#include <QVariantAnimation>

class QTextDocument;
class WaitingSpinnerWidget;

class RunProgressBar final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(RunProgressBar)

public:
    explicit RunProgressBar(QWidget* parent = nullptr);

    int progress() const;
    void setProgress(int progress);

    QColor color() const;
    void setColor(const QColor& color);

    QString toHtml(const QByteArray& encoding = QByteArray()) const;
    void setHtml(const QString& html);

    QString toPlainText() const;
    void setPlainText(const QString& text);

    bool isBusy() const;
    void setBusy(bool busy);

private:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    bool m_progressVisible;
    int m_progress;
    QColor m_color;
    QVariantAnimation m_springAnimation;
    QVariantAnimation m_faderAnimation;
    QTextDocument* m_document;
    WaitingSpinnerWidget* m_busyIndicator;
};

#endif // RUNPROGRESSBAR_H