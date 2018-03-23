#ifndef BUTTONSLICE_H
#define BUTTONSLICE_H

#include <QList>
#include <QWidget>

class QLayout;
class QPushButton;

class ButtonSlice : public QWidget
{
        Q_OBJECT

    private:
        struct ButtonElement {
            int id;
            QColor topColor;
            QColor bottomColor;
            QPushButton* button;
        };

    public:
        struct Settings {
            /* Sizes */
            qreal cellWidth;
            qreal borderRadius;
        };

    public:
        explicit ButtonSlice(QWidget* parent = nullptr);
        void add(int id, const QColor& topColor, const QColor& bottomColor);
        QPushButton* get(int id);
        Settings& settings();

    public slots:
        void triggerSettings();

    protected:
        QSize sizeHint() const override;
        void paintEvent(QPaintEvent* event) override;

    private:
        QLayout* _layout;
        Settings _settings;
        QList<ButtonElement> _elements;
};

#endif // BUTTONSLICE_H