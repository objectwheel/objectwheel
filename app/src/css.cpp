#include <css.h>
#include <fit.h>
#include <QColor>

#define CSS_SPINBOX "\
QAbstractSpinBox { \
	selection-background-color: #555555; \
	selection-color: white; \
	background:white; \
	border: %1 solid #bdbebf; \
	border-radius:%2; \
} QAbstractSpinBox::up-button { \
	subcontrol-position: top right; \
	subcontrol-origin: margin; \
	position: absolute; \
	top: 0px; right: 0px; bottom: 0px; \
	width: %3; \
    background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 %6, stop:1 %7); \
	border-top-right-radius:%4; \
	border-bottom-right-radius:%4; \
} QAbstractSpinBox::up-button:pressed { \
	background:#888888; \
} QAbstractSpinBox::up-button:disabled { \
	background:grey; \
} QAbstractSpinBox::up-arrow { \
	image: url(:/resources/images/plus.png); \
	width: %5; \
	height: %5; \
} QAbstractSpinBox::down-button { \
	subcontrol-position: top left; \
	subcontrol-origin: margin; \
	position: absolute; \
	top: 0px; left: 0px; bottom: 0px; \
	width: %3; \
    background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 %8, stop:1 %9); \
	border-top-left-radius:%4;	 \
	border-bottom-left-radius:%4; \
} QAbstractSpinBox::down-button:pressed { \
	background:#888888; \
} QAbstractSpinBox::down-button:disabled { \
	background:grey; \
} QAbstractSpinBox::down-arrow { \
	image: url(:/resources/images/minus.png); \
	width: %5; \
	height: %5; \
}"

#define CSS_LINEEDIT "\
QLineEdit {\
	selection-background-color: #555555; \
	selection-color: white; \
	background:white; \
	border: none; \
	border-radius:%2;\
	padding:%2;\
}"

#define CSS_PROPERTY_ITEM "\
#propertyItem {\
	background-color: rgba(0, 0, 0, 60);\
	border: 0px;\
	border-radius: %1px;\
	margin-right: %2px;\
	margin-bottom: %3px;\
}"

#define CSS_TOOLBAR "\
QToolBar { \
    padding-left: %1px; \
    spacing: %2px; \
    background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 %3, stop:1 %4); \
	border: none;\
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

#define CSS_TOOLBOX_BUTTON "\
QRadioButton::indicator { \
	width: %1px; \
	height: %1px; \
} QRadioButton::indicator::checked { \
	border-image: url(:/resources/images/toolboxbuttonfilled.png); \
} QRadioButton::indicator::unchecked { \
	border-image: url(:/resources/images/toolboxbutton.png); \
}"

#define CSS_PROPERTIES_BUTTON "\
QRadioButton::indicator { \
	width: %1px; \
	height: %1px; \
} QRadioButton::indicator::checked { \
	border-image: url(:/resources/images/propertiesbuttonfilled.png); \
} QRadioButton::indicator::unchecked { \
	border-image: url(:/resources/images/propertiesbutton.png); \
}"

#define CSS_FORMS_BUTTON "\
QRadioButton::indicator { \
	width: %1px; \
	height: %1px; \
} QRadioButton::indicator::checked { \
    border-image: url(:/resources/images/formsbuttonfilled.png); \
} QRadioButton::indicator::unchecked { \
    border-image: url(:/resources/images/formsbutton.png); \
}"

#define CSS_FORMS_LISTWIDGET \
"QListView {\
    background:#52616D;\
    padding: %1px;\
	border: 1px solid #465563;\
	border-radius: %2px;\
} QListView::item {\
	color:white;\
	border: none;\
	border-radius: %2px;\
	padding: %2px;\
	margin: %2px;\
} QListView::item:selected {\
	background: white;\
	color: black\
}"

using namespace Fit;

QString CSS::SpinBox;
QString CSS::LineEdit;
QString CSS::PropertyItem;
QString CSS::Toolbar;
QString CSS::DesignerToolbar;
QString CSS::DesignerToolbarV;
QString CSS::ToolboxButton;
QString CSS::PropertiesButton;
QString CSS::FormsButton;
QString CSS::FormsListWidget;

void CSS::init()
{
    /* SpinBox */
	SpinBox = QString(CSS_SPINBOX).arg(fit(1)).arg(fit(5))
                  .arg(fit(30)).arg(fit(4)).arg(fit(15)).arg(QColor("#6BB64B").name()).arg(QColor("#6BB64B").darker(115).name())
            .arg(QColor("#C61717").name()).arg(QColor("#C61717").darker(115).name());

	/* LineEdit */
	LineEdit = QString(CSS_LINEEDIT).arg(fit(2));

	/* PropertyItem */
	PropertyItem = QString(CSS_PROPERTY_ITEM).arg(fit(4)).arg(fit(2)).arg(fit(5));

	/* Toolbar */
    Toolbar = QString(CSS_TOOLBAR).arg(fit(12)).arg(fit(2)).arg(QColor("#6BB64B").name()).arg(QColor("#6BB64B").darker(115).name());

    /* DesignerToolbar */
    DesignerToolbar = QString(CSS_DESIGNER_TOOLBAR).arg(QColor("#ABAFB2").lighter(110).name()).arg(QColor("#EAEEF1").name()).arg(QColor("#d0d4d7").name());

    /* DesignerToolbarV */
    DesignerToolbarV = QString(CSS_DESIGNER_TOOLBAR_V).arg(QColor("#ABAFB2").lighter(110).name()).arg(QColor("#EAEEF1").name()).arg(QColor("#d0d4d7").name());

    /* ToolboxButton */
    ToolboxButton = QString(CSS_TOOLBOX_BUTTON).arg(fit(20));

	/* PropertiesButton */
    PropertiesButton = QString(CSS_PROPERTIES_BUTTON).arg(fit(20));

    /* FormsButton */
    FormsButton = QString(CSS_FORMS_BUTTON).arg(fit(20));

    /* FormsListWidget */
    FormsListWidget = QString(CSS_FORMS_LISTWIDGET).arg(fit(5)).arg(fit(2));
}
