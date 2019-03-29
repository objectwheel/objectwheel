#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <QProgressBar>
#include <QVariantAnimation>

class ProgressBar final : public QProgressBar
{
    Q_OBJECT
    Q_DISABLE_COPY(ProgressBar)

public:
    struct Settings {
        /* Color settings */
        QColor backgroundColor;
        QColor borderColor;
        QColor chunkColor;
        QColor indeterminateColor;
        /* Size settings */
        qreal borderRadius;
    };

public:
    explicit ProgressBar(QWidget* parent = nullptr);

    bool isIndeterminate() const;
    void setIndeterminate(bool indeterminate);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void triggerSettings();

private:
    void paintEvent(QPaintEvent*) override;

private:
    Settings m_settings;
    bool m_indeterminate;
    QVariantAnimation m_indeterminateAnim;
};

#endif // PROGRESSBAR_H
