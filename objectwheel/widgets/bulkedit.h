#ifndef BULKEDIT_H
#define BULKEDIT_H

#include <QList>
#include <QLineEdit>

class BulkEdit final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(BulkEdit)

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
        for (const LineElement& element : m_elements) {
            if (element.id == id)
                return static_cast<WidgetPtr>(element.edit);
        }
        return nullptr;
    }

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void triggerSettings();

private:
    void paintEvent(QPaintEvent* event) override;

signals:
    void returnPressed();

private:
    QLayout* m_layout;
    Settings m_settings;
    QList<LineElement> m_elements;
};

#endif // BULKEDIT_H