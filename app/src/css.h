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
        static QString DesignerToolbar;
        static QString DesignerToolbarV;
};

#endif // CSS_H
