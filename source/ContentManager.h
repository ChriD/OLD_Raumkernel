#ifndef CONTENTMANAGER_H_INCLUDED
#define CONTENTMANAGER_H_INCLUDED


#include "os.h"

#include <boost/cstdint.hpp>
#include <boost/signals2.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/fusion/include/map.hpp>

#include <rapidxml/rapidxml.hpp>

#include "Manager.h"
#include "MediaItem.h"

using namespace ApplicationLogging;
using namespace rapidxml;


namespace Raumkernel
{

	typedef boost::signals2::signal<void(std::string)> typeSignalListContentChanged;

	class ContentManager : public HTTPRequestManager
	{
		public:
			EXPORT ContentManager();
			EXPORT virtual ~ContentManager();

			EXPORT void Lock();
			EXPORT void UnLock();
			
			EXPORT void SubscribeToMediaServer();

			EXPORT void StartGetMediaItemListByContainerId(std::string _containerId, std::string _searchCriteria = "", std::string _listId = "", bool _isZonePlaylist = false, bool _sync = false);
			EXPORT void StartGetMediaItemListByZoneUDN(std::string _zoneUDN, bool _sync = false);
			EXPORT void StartGetMediaItemListsByContainerUpdateIds(std::string _containerUpdateIds, bool _sync = false);
			EXPORT void SubscribeSignalListContentChanged(const typeSignalListContentChanged::slot_type &_subscriber);
			EXPORT std::list<MediaItem> EndGetMediaItemList(std::string _listId);						
			EXPORT bool IsListInCache(std::string _listId);
			EXPORT boost::uint32_t GetLastUpdateIdForList(std::string _listId);
			EXPORT void SetLastUpdateIdForList(std::string _listId, boost::uint32_t);

			EXPORT MediaItem CreateMediaItemFromCurrentTrackMetadata(std::string _trackMetadata);
		
		protected:

			void LockListForUpdate(bool _lock);
			void CreateListFromResultXML(std::string _result, std::string _extraData);
			void ListReady(std::string _listId, bool _zonePlaylist = false);
			MediaItem CreateMediaItemFromXMLNode(xml_node<> *_xmlNode);
				

		private:
			typeSignalListContentChanged		signalListContentChanged;
			boost::mutex						mutexListUpdate;

			boost::unordered_map<std::string, std::list<MediaItem>>	listCache;
			boost::unordered_map<std::string, boost::uint32_t>	listUpdateId;

			void OnSearchEnded(std::string _result, boost::uint32_t _numberReturned, boost::uint32_t _totalMatches, boost::uint32_t _updateId, std::string _extraData);
			void OnBrowseEnded(std::string _result, boost::uint32_t _numberReturned, boost::uint32_t _totalMatches, boost::uint32_t _updateId, std::string _extraData);
				
	};




}

#endif // CONTENTMANAGER_H_INCLUDED
