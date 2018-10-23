#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <QProgressBar>
#include <QVariantAnimation>

class ProgressBar : public QProgressBar
{
    Q_OBJECT
    Q_PROPERTY(bool indeterminate READ indeterminate WRITE setIndeterminate)

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
    void triggerSettings();
    void setIndeterminate(bool indeterminate);
    bool indeterminate() const;

protected:
    void paintEvent(QPaintEvent*) override;

private:
    Settings m_settings;
    bool m_indeterminate;
    QVariantAnimation m_indeterminateAnim;
};

#endif // PROGRESSBAR_H
