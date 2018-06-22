#ifndef CSS_H
#define CSS_H

#define CSS_SCROLLBAR "\
QScrollBar:vertical { \
    background: #ececec; \
    width: 6px; \
} QScrollBar::handle:vertical { \
    background: #909090; \
    min-height: 15px; \
    border-radius: 3px; \
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
    height: 6px; \
} QScrollBar::handle:horizontal { \
    background: #909090; \
    min-width: 15px; \
    border-radius: 3px; \
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
    border-top: 1px solid #D0D0D0;\
    border-bottom: 1px solid #D0D0D0;\
    background: #e4e4e4; \
    spacing: 5px; \
    padding: 0px; \
}"

#define CSS_DESIGNER_TOOLBAR_V "\
QToolBar { \
    border-left: 1px solid #D0D0D0;\
    border-right: 1px solid #D0D0D0;\
    background: #e4e4e4; \
    spacing: 5px; \
    padding: 0px; \
}"

#define CSS_DESIGNER_PINBAR "\
QToolBar { \
    border-top: 1px solid  #c6c6c6;\
    border-bottom: 1px solid #c6c6c6;\
    border-right: 1px solid #c6c6c6;\
    border-left: 3px solid #0D74C8;\
    margin-left: 1px; margin-right: 1px;\
    background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 #ffffff, stop:1 #e3e3e3); \
    spacing: 5px; \
}"

#endif // CSS_H
