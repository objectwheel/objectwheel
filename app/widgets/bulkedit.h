#ifndef BULKEDIT_H
#define BULKEDIT_H

#include <QList>
#include <QLineEdit>

class QLayout;

class BulkEdit : public QWidget
{
    Q_OBJECT

private:
    struct LineElement {
        int id;
        QString text;
        QWidget* edit;
    };

public:
    struct Settings {
        /* Colors */
        QColor borderColor;
        QColor backgroundColor;
        QColor labelColor;

        /* Sizes */
        qreal cellHeight;
        qreal borderRadius;
        int leftMargin;
        int rightMargin;
    };

public:
    explicit BulkEdit(QWidget* parent = nullptr);
    void add(int id, const QString& label, QWidget* widget = new QLineEdit);
    Settings& settings();

    template <typename WidgetPtr> WidgetPtr get(int id)
    {
        for (const LineElement& element : _elements) {
            if (element.id == id)
                return static_cast<WidgetPtr>(element.edit);
        }
        return nullptr;
    }

public slots:
    void triggerSettings();

protected:
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void paintEvent(QPaintEvent* event) override;

private:
    QLayout* _layout;
    Settings _settings;
    QList<LineElement> _elements;
};

#endif // BULKEDIT_H