#ifndef PROPERTYITEM_H
#define PROPERTYITEM_H

#include <QWidget>
#include <QPointer>

class Control;

class PropertyItem : public QWidget
{
        Q_OBJECT
    private:
        QPointer<Control> _control;
        QString _property;
        bool _valid;

    public:
        explicit PropertyItem(Control* control, const QString& property, QWidget *parent = 0);
        Control* control() const;
        QString property() const;
        bool isValid() const { return _valid; }

    protected slots:
        void fillCup();
        void applyFont(const QFont& font);
        void applyValue(const QVariant& value);

    protected:
        bool eventFilter(QObject* o, QEvent* e);
        void paintEvent(QPaintEvent *e);

    signals:
        void valueApplied();
};

#endif // PROPERTYITEM_H
