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
	background:#98d367; \
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
	background:#ed5f55; \
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
	border: %1 solid #bdbebf; \
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

using namespace Fit;

QString CSS::SpinBox;
QString CSS::LineEdit;
QString CSS::PropertyItem;

void CSS::init()
{
	/* SpinBox */
	SpinBox = QString(CSS_SPINBOX).arg(fit(1)).arg(fit(5))
				  .arg(fit(30)).arg(fit(4)).arg(fit(15));

	/* LineEdit */
	LineEdit = QString(CSS_LINEEDIT).arg(fit(1)).arg(fit(5));

	/* PropertyItem */
	PropertyItem = QString(CSS_PROPERTY_ITEM).arg(fit(4)).arg(fit(2)).arg(fit(5));
}
