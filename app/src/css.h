#ifndef CSS_H
#define CSS_H

#include <QString>

class CSS
{
	private:
		CSS() { }

	public:
		static void init();

		static QString SpinBox;
		static QString LineEdit;
		static QString PropertyItem;
		static QString Toolbar;
		static QString ToolboxButton;
		static QString PropertiesButton;
		static QString BindingButton;
		static QString PagesButton;
		static QString BindingListWidget;
};

#endif // CSS_H
