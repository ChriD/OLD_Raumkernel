#include "MediaBrowser.h"

#include "ContentManager.h"

namespace Raumkernel
{

	MediaBrowser::MediaBrowser(std::string _browserId) : RaumkernObject()
	{
		browserId = _browserId;
		managerList.contentManager->SubscribeSignalListContentChanged(typeSignalListContentChanged::slot_type(boost::bind(&MediaBrowser::BrowseReady, this, _1)));
	}

	MediaBrowser::~MediaBrowser()
	{
	}
	
	void MediaBrowser::SubscribeSignalBrowserListReady(const typeSignalBrowserListReady::slot_type &_subscriber)
	{
		signalBrowserListReady.connect(_subscriber);
	}


	void MediaBrowser::BrowseReady(std::string _listId)
	{
		// if list we are waiting for is ready , do this!. There may be other lists which get ready but twe wont mind about		
		if (curWaitingListId == _listId)
		{
			curWaitingListId = "";
			signalBrowserListReady(_listId, browserId);
		}		
	}


	void MediaBrowser::BrowseTo(std::string _listId, bool _tryReadFromCache)
	{
		curWaitingListId = _listId;
		managerList.contentManager->StartGetMediaItemListByContainerId(_listId, "", "", false, false);
		pathList.push_back(_listId);
	}

	void MediaBrowser::BrowseTo(MediaItem _mediaItem, bool _tryReadFromCache)
	{
		this->BrowseTo(_mediaItem.id, _tryReadFromCache);
	}

	void MediaBrowser::BrowseBack(bool _tryReadFromCache)
	{
		std::string listId;

		// if we have more than 2 path entrys we can nump back, otherwise we are on the first one and we cant!
		if (pathList.size() > 1)
		{
			/*
			for (auto i = pathList.begin(), j = --pathList.end(); i != j; ++i) 
			{
				listId = *i;
			}
			*/
			
			pathList.pop_back();
			listId = pathList.back();
			this->BrowseTo(listId, _tryReadFromCache);
		}		
	}

	std::list<MediaItem> MediaBrowser::GetCurBrowserList()
	{
		std::string listId = pathList.back();		
		return managerList.contentManager->EndGetMediaItemList(listId);
	}


}

