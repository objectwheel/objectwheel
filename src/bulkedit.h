#ifndef BULKEDIT_H
#define BULKEDIT_H

#include <QList>
#include <QWidget>

class QLayout;
class QLineEdit;

class BulkEdit : public QWidget
{
        Q_OBJECT

    private:
        struct LineElement {
            int id;
            QString text;
            QLineEdit* edit;
        };

    public:
        struct Settings {
            /* Colors */
            QColor borderColor;
            QColor backgroundColor;
            QColor labelColor;
            QColor textColor;

            /* Sizes */
            qreal cellHeight;
            qreal borderRadius;
            int leftMargin;
            int rightMargin;
        };

    public:
        explicit BulkEdit(QWidget* parent = nullptr);
        Settings& settings();
        QLineEdit* get(int id);
        void add(int id, const QString& label);

    public slots:
        void triggerSettings();

    protected:
        void paintEvent(QPaintEvent* event) override;
        QSize sizeHint() const override;

    private:
        QLayout* _layout;
        Settings _settings;
        QList<LineElement> _elements;
};

#endif // BULKEDIT_H