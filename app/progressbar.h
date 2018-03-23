#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <QProgressBar>

class ProgressBar : public QProgressBar
{
        Q_OBJECT

    public:
        struct Settings {
                /* Color settings */
                QColor backgroundColor;
                QColor borderColor;
                QColor chunkColor;
                /* Size settings */
                qreal borderRadius;
        };

    public:
        explicit ProgressBar(QWidget* parent = nullptr);
        void triggerSettings();

    protected:
        void paintEvent(QPaintEvent*) override;

    private:
        Settings m_settings;
};

#endif // PROGRESSBAR_H
