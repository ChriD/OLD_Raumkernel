#include "MediaItem.h"


namespace Raumkernel
{

	MediaItem::MediaItem() : RaumkernObject()
	{
		id = "";
		parentId = "";
		upnpClass = "";
		raumfeldName = "";
		raumfeldSection = "";
		artist = "";
		artistArtUri = "";
		album = "";
		albumArtUri = "";
		title = "";
		duration = "";
		res = "";
	}

	MediaItem::~MediaItem()
	{
	}


}

