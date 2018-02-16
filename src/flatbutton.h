#ifndef FLATBUTTON_H
#define FLATBUTTON_H

#include <QPushButton>

class QGraphicsDropShadowEffect;

class FlatButton : public QPushButton
{
		Q_OBJECT

    public:
        struct Settings {
            /* Colors */
            QColor topColor;
            QColor bottomColor;
            QColor textColor;
            /* Sizes */
            qreal borderRadius;
            /* Others */
            bool iconButton;
        };

	public:
		explicit FlatButton(QWidget *parent = 0);
        Settings& settings();

    public slots:
        void triggerSettings();

	protected:
        void paintEvent(QPaintEvent* event) override;
        QSize sizeHint() const override;

    private:
        Settings _settings;
};

#endif // FLATBUTTON_H
