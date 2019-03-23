#ifndef RUNPROGRESSBAR_H
#define RUNPROGRESSBAR_H

#include <QWidget>
#include <QTextLine>
#include <QVariantAnimation>

class QTextDocument;
class WaitingSpinnerWidget;

class RunProgressBar final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(RunProgressBar)

public:
    explicit RunProgressBar(QWidget* parent = nullptr);

    bool isBusy() const;
    void setBusy(bool busy);

    int progress() const;
    void setProgress(int progress);

    Qt::TextFormat textFormat() const;
    void setTextFormat(Qt::TextFormat format);

    QString text() const;
    void setText(const QString& text);

    QColor progressColor() const;
    void setProgressColor(const QColor& color);

private:
    void updateLine();
    void updateToolTip();
    void updateDocument();

    int paddingWidth() const;
    int maximumTextWidth() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void changeEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    int m_progress;
    Qt::TextFormat m_textFormat;
    QString m_text;
    QColor m_progressColor;

    bool m_progressVisible;
    int m_widerLineWidth;
    QTextLine m_line;
    QVariantAnimation m_springAnimation;
    QVariantAnimation m_faderAnimation;
    QTextDocument* m_document;
    WaitingSpinnerWidget* m_busyIndicator;
};

#endif // RUNPROGRESSBAR_H