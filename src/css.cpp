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

#define CSS_SCROLLBAR_H "\
QScrollBar:horizontal { \
    background: transparent; \
    height: %2px; \
} QScrollBar::handle:horizontal { \
    background: #909497; \
    min-width: %1px; \
    border-radius: %3px; \
} QScrollBar::add-line:horizontal { \
    background: none; \
} QScrollBar::sub-line:horizontal { \
    background: none; \
} QScrollBar::left-arrow:horizontal, QScrollBar::right-arrow:horizontal { \
    background: none; \
} QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { \
    background: none; \
}"

#define CSS_DESIGNER_TOOLBAR "\
QToolBar { \
    border-top: 1px solid %1;\
    border-bottom: 1px solid %1;\
    background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 %2, stop:1 %3); \
    spacing: %4px; \
}"

#define CSS_DESIGNER_TOOLBAR_V "\
QToolBar { \
    border-left: 1px solid %1;\
    border-right: 1px solid %1;\
    background: qlineargradient(spread:pad, x1:1, y1:0.5, x2:0, y2:0.5, stop:1 %2, stop:0 %3); \
    spacing: %4px; \
}"

#define CSS_DESIGNER_PINBAR "\
QToolBar { \
    border-top: 1px solid  %1;\
    border-bottom: 1px solid %1;\
    border-right: 1px solid %1;\
    border-left: %6px solid #0D74C8;\
    margin-left: %5px; margin-right: %5px;\
    margin-top: %5px;\
    background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.8, stop:0 %2, stop:1 %3); \
    spacing: %4px; \
}"

QString CSS::ScrollBar;
QString CSS::ScrollBarH;
QString CSS::DesignerToolbar;
QString CSS::DesignerToolbarV;
QString CSS::DesignerPinbar;

void CSS::init()
{
    /* ScrollBar */
    ScrollBar = QString(CSS_SCROLLBAR).arg(fit::fx(15)).arg(fit::fx(6)).arg(fit::fx(2.5));

    /* ScrollBarH */
    ScrollBarH = QString(CSS_SCROLLBAR_H).arg(fit::fx(15)).arg(fit::fx(6)).arg(fit::fx(2.5));

    /* DesignerToolbar */
    DesignerToolbar = QString(CSS_DESIGNER_TOOLBAR).arg(QColor("#ABAFB2").lighter(110).
      name()).arg(QColor("#EAEEF1").name()).arg(QColor("#d0d4d7").name()).arg(fit::fx(5));

    /* DesignerToolbarV */
    DesignerToolbarV = QString(CSS_DESIGNER_TOOLBAR_V).arg(QColor("#ABAFB2").lighter(110).
      name()).arg(QColor("#EAEEF1").name()).arg(QColor("#d0d4d7").name()).arg(fit::fx(5));

    /* DesignerPinbar */
    DesignerPinbar = QString(CSS_DESIGNER_PINBAR).arg(QColor("#ABAFB2").lighter(110).
      name()).arg(QColor("#F7FBFF").name()).arg(QColor("#DCDFE2").name())
            .arg(fit::fx(5)).arg(fit::fx(1.8)).arg(fit::fx(3.5));

}
