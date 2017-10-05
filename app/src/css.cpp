#include <css.h>
#include <fit.h>
#include <QColor>

#define CSS_SCROLLBAR "\
QScrollBar:vertical { \
    background: transparent; \
    width: %2px; \
} QScrollBar::handle:vertical { \
    background: #909497; \
    min-height: %1px; \
    border-radius: %3px; \
} QScrollBar::add-line:vertical { \
    background: none; \
} QScrollBar::sub-line:vertical { \
    background: none; \
} QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical { \
    background: none; \
} QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { \
    background: none; \
}"

#define CSS_DESIGNER_TOOLBAR "\
QToolBar { \
    border-bottom: 1px solid %1;\
    background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 %2, stop:1 %3); \
}"

#define CSS_DESIGNER_TOOLBAR_V "\
QToolBar { \
    border-right: 1px solid %1;\
    background: qlineargradient(spread:pad, x1:1, y1:0.5, x2:0, y2:0.5, stop:1 %2, stop:0 %3); \
}"

using namespace Fit;

QString CSS::ScrollBar;
QString CSS::DesignerToolbar;
QString CSS::DesignerToolbarV;

void CSS::init()
{
    /* ScrollBar */
    ScrollBar = QString(CSS_SCROLLBAR).arg(fit(15)).arg(fit(6)).arg(fit(3));

    /* DesignerToolbar */
    DesignerToolbar = QString(CSS_DESIGNER_TOOLBAR).arg(QColor("#ABAFB2").lighter(110).name()).arg(QColor("#EAEEF1").name()).arg(QColor("#d0d4d7").name());

    /* DesignerToolbarV */
    DesignerToolbarV = QString(CSS_DESIGNER_TOOLBAR_V).arg(QColor("#ABAFB2").lighter(110).name()).arg(QColor("#EAEEF1").name()).arg(QColor("#d0d4d7").name());
}
