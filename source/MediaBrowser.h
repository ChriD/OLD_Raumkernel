#ifndef MEDIABROWSER_H_INCLUDED
#define MEDIABROWSER_H_INCLUDED


#include "os.h"
#include "RaumkernObject.h"
#include "MediaItem.h"
#include "Manager.h"

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

	typedef boost::signals2::signal<void(std::string, std::string)> typeSignalBrowserListReady;

	class MediaBrowser : public RaumkernObject
	{
		public:
			EXPORT MediaBrowser(std::string _browserId);
			EXPORT virtual ~MediaBrowser();	

			EXPORT void BrowseTo(std::string _listId, bool _tryReadFromCache = false);
			EXPORT void BrowseTo(MediaItem _mediaItem, bool _tryReadFromCache = false);
			EXPORT void BrowseBack(bool _tryReadFromCache = true);
			EXPORT std::list<MediaItem> GetCurBrowserList();

			EXPORT void SubscribeSignalBrowserListReady(const typeSignalBrowserListReady::slot_type &_subscriber);

		protected:
			// stores the "path" of the browse
			std::list<std::string> pathList;

			void BrowseReady(std::string _listId);

		private:			
			std::string browserId;
			std::string curWaitingListId;

			typeSignalBrowserListReady signalBrowserListReady;
	};




}

#endif // MEDIABROWSER_H_INCLUDED
