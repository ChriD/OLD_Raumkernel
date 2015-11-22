#ifndef MANAGERLIST_H
#define MANAGERLIST_H


namespace Raumkernel
{

	class DeviceManager;
	class ZoneManager;
	class AliveCheckManager;
	class ContentManager;

	class ManagerList
	{
		public:
			ManagerList();
			virtual ~ManagerList();
			DeviceManager		*deviceManager;
			ZoneManager			*zoneManager;
			AliveCheckManager	*aliveCheckManager;		
			ContentManager		*contentManager;
	};


}

#endif // MANAGERLIST_H