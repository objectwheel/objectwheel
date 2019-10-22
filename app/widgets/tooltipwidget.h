#ifndef TOOLTIPWIDGET_H
#define TOOLTIPWIDGET_H

#include <QEvent>
#include <QApplication>

#include <private/qwidgettextcontrol_p.h>
#include <private/qtextimagehandler_p.h>

class TextImageHandler final : public QTextImageHandler
{
public:
    explicit TextImageHandler(QObject* parent = nullptr);
    void drawObject(QPainter* p, const QRectF& r, QTextDocument* d, int o, const QTextFormat& f) override;
};

template <typename Class>
class ToolTipWidget : public Class
{
public:
    explicit ToolTipWidget(QWidget* parent = nullptr) : Class(parent) {}
private:
    bool event(QEvent* e) {
        bool ret = Class::event(e);
        if (e->type() == QEvent::ToolTip) {
            const QWidgetList& topLevelWidgets = QApplication::topLevelWidgets();
            for (QWidget* widget : topLevelWidgets) {
                if (widget->inherits("QTipLabel")) {
                    if (auto control = widget->findChild<QWidgetTextControl*>()) {
                        QAbstractTextDocumentLayout* layout = control->document()->documentLayout();
                        if (auto object = dynamic_cast<QObject*>(layout->handlerForObject(QTextFormat::ImageObject))) {
                            if (!object->inherits("TextImageHandler"))
                                layout->registerHandler(QTextFormat::ImageObject, new TextImageHandler(layout));
                        }
                    }
                    break;
                }
            }
        }
        return ret;
    }
};

#endif // TOOLTIPWIDGET_H
