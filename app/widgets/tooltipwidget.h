#ifndef TOOLTIPWIDGET_H
#define TOOLTIPWIDGET_H

#include <textimagehandler.h>

#include <QEvent>
#include <QApplication>

#include <private/qwidgettextcontrol_p.h>

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
