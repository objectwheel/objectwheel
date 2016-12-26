#include <css.h>
#include <fit.h>

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
	background:#1e8145; \
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
	background:#c03638; \
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
	spacing: %1px; \
	background: #1e8145; \
	border-top: %2px solid #555555; \
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

#define CSS_BINDING_BUTTON "\
QRadioButton::indicator { \
	width: %1px; \
	height: %1px; \
} QRadioButton::indicator::checked { \
	border-image: url(:/resources/images/bindingbuttonfilled.png); \
} QRadioButton::indicator::unchecked { \
	border-image: url(:/resources/images/bindingbutton.png); \
}"

#define CSS_BINDING_LISTWIDGET \
"QListView {\
	background:#566573;\
	padding: %1px;\
	border: 1px solid #485365;\
	border-radius: %2px;\
} QListView::item {\
	color:white;\
	border: none;\
	border-radius: %2px;\
} QListView::item:selected {\
	background: white;\
	color: black\
}"

#define CSS_COMBOBOX "\
QComboBox {\
	border: none;\
	border-top-left-radius: %1px;\
	border-bottom-left-radius: %1px\
} QComboBox::drop-down {\
	width:0px;\
	height: 0px;\
}"

using namespace Fit;

QString CSS::SpinBox;
QString CSS::LineEdit;
QString CSS::PropertyItem;
QString CSS::Toolbar;
QString CSS::ToolboxButton;
QString CSS::PropertiesButton;
QString CSS::BindingButton;
QString CSS::BindingListWidget;
QString CSS::ComboBox;

void CSS::init()
{
	/* SpinBox */
	SpinBox = QString(CSS_SPINBOX).arg(fit(1)).arg(fit(5))
				  .arg(fit(30)).arg(fit(4)).arg(fit(15));

	/* LineEdit */
	LineEdit = QString(CSS_LINEEDIT).arg(fit(2));

	/* PropertyItem */
	PropertyItem = QString(CSS_PROPERTY_ITEM).arg(fit(4)).arg(fit(2)).arg(fit(5));

	/* Toolbar */
	Toolbar = QString(CSS_TOOLBAR).arg(fit(5)).arg(fit(1));

	/* ToolboxButton */
	ToolboxButton = QString(CSS_TOOLBOX_BUTTON).arg(fit(32));

	/* PropertiesButton */
	PropertiesButton = QString(CSS_PROPERTIES_BUTTON).arg(fit(32));

	/* BindingButton */
	BindingButton = QString(CSS_BINDING_BUTTON).arg(fit(32));

	/* BindingListWidget */
	BindingListWidget = QString(CSS_BINDING_LISTWIDGET).arg(fit(5)).arg(fit(2));

	/* ComboBox */
	ComboBox = QString(CSS_COMBOBOX).arg(fit(2));
}
