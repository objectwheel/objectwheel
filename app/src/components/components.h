#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <apiai.h>
#include <audiorecorder.h>
#include <audioplayer.h>
#include <aispeak.h>
#include <documenthandler.h>
#include <filemanager.h>
#include <qquickfolderlistmodel.h>

class Components
{
	private:
		Components() { }

	public:
		static void init();
};


#endif // COMPONENTS_H
