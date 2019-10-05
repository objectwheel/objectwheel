#ifndef EDITORWIDGETCACHE_H
#define EDITORWIDGETCACHE_H

#include <QHash>
#include <QObject>

class QWidget;
template<class T>
class QStack;

class EditorWidgetCache final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(EditorWidgetCache)

public:
    enum Type {
        String,
        Enum,
        Bool,
        Color,
        Int,
        Real,
        FontSize,
        FontFamily,
        FontWeight,
        FontCapitalization
    };
    Q_ENUM(Type)

public:
    explicit EditorWidgetCache(QObject* parent = nullptr);
    ~EditorWidgetCache() override;

    void clear();
    void reserve(int size);
    void push(Type type, QWidget* widget);
    QWidget* pop(Type type);

private:
    QWidget* createWidget(Type type);

private:
    using PropertyStack = QStack<QWidget*>;
    using PropertyHash = QHash<Type, PropertyStack*>;

    PropertyHash m_widgets;
};

#endif // EDITORWIDGETCACHE_H
