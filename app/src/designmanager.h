#ifndef DESIGNMANAGER_H
#define DESIGNMANAGER_H

#include <QObject>
#include <QUrl>
#include <QRect>
#include <QList>

class DesignManagerPrivate;
class QWidget;

class Control {
    public: // variables
        QString id;
        QUrl url;
        QRect boundingRect;
        Control* parent = nullptr;
        QList<Control*> children;

    public: // functions
        QList<Control*> findChildren(const QString& id, Qt::FindChildOptions option = Qt::FindChildrenRecursively) const;

    private:
        QList<Control*> findChildrenRecursively(const QString& id, QList<Control*> parent) const;
};

class Item : Control { };

class Page : Control {
    public: // variables
        bool mainPage = false;
};

class DesignManager : public QObject
{
        Q_OBJECT
        Q_DISABLE_COPY(DesignManager)

    private:
        static DesignManagerPrivate* _d;

    public:
        explicit DesignManager(QObject *parent = 0);
        static DesignManager* instance();
        ~DesignManager();
        static void setSettleWidget(QWidget* widget);
        static void showWidget();
        static void hideWidget();

};

#endif // DESIGNMANAGER_H
