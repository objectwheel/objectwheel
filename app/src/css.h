#ifndef CSS_H
#define CSS_H

#include <QString>

class CSS
{
	private:
		CSS() { }

	public:
		static void init();

        static QString ScrollBar;
        static QString ScrollBarH;
        static QString DesignerToolbar;
        static QString DesignerToolbarV;
        static QString DesignerPinbar;
};

#endif // CSS_H
