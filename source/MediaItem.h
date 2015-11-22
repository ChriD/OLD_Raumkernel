#ifndef MEDIAITEM_H_INCLUDED
#define MEDIAITEM_H_INCLUDED


#include "os.h"
#include "RaumkernObject.h"

#include <boost/cstdint.hpp>
#include <boost/signals2.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/fusion/include/map.hpp>

#include <rapidxml/rapidxml.hpp>

using namespace ApplicationLogging;
using namespace rapidxml;


namespace Raumkernel
{
	
	enum class MediaItemType {	
		MIT_TRACKCONTAINER = 0, 
		MIT_ALBUM = 1,
		MIT_ARTIST = 2, 
		MIT_GENRE = 3, 
		MIT_COMPOSER = 4, 
		MIT_CONTAINER = 5,
		MIT_RADIO = 6,
		MIT_PLAYLIST = 7,
		MIT_SHUFFLE = 8,
		MIT_RHAPSODYRADIO = 9,
		MIT_STORAGEFOLDER = 10,
		MIT_LINEIN = 11,
		MIT_TRACK = 12
	};

	

	class MediaItem : public RaumkernObject
	{
		public:
			MediaItem();
			virtual ~MediaItem();

			MediaItemType type;

			std::string id;
			std::string parentId;		
			std::string upnpClass;
			std::string raumfeldName;
			std::string raumfeldSection;
			std::string res;

			// MIT_ARTIST
			std::string artist;
			std::string artistArtUri;

			// MIT_ALBUM
			std::string album;
			std::string albumArtUri;

			// MIT_TRACK
			std::string title;
			std::string duration;

		protected:

		private:		
	};
	

}

#endif // MEDIAITEM_H_INCLUDED
