/*
This file holds all methods from Raumkern which have to do with signal
*/

#include "Raumkern.h"

namespace Raumkernel
{

	// Raumkern::SubscribeSignalSystemShutdown
	void Raumkern::SubscribeSignalSystemShutdown(const typeSignalSystemShutdown::slot_type &_subscriber)
	{
		signalSystemShutdown.connect(_subscriber);
	}


	// Raumkern::SubscribeSignalSystemReady
	void Raumkern::SubscribeSignalSystemReady(const typeSignalSystemReady::slot_type &_subscriber)
	{
		signalSystemReady.connect(_subscriber);
	}


	// Raumkern::SubscribeSignalZoneConfigurationChanged
	void Raumkern::SubscribeSignalZoneConfigurationChanged(const typeSignalZoneConfigurationChanged::slot_type &_subscriber)
	{
		if (managerList.zoneManager)
			managerList.zoneManager->SubscribeSignalZoneConfigurationChanged(_subscriber);
	}


	// Raumkern::SubscribeSignalDeviceListChanged
	void Raumkern::SubscribeSignaDeviceListChanged(const typeSignalDeviceListChanged::slot_type &_subscriber)
	{
	
		if (managerList.deviceManager)
			managerList.deviceManager->SubscribeSignalDeviceListChanged(_subscriber);
	}

	// Raumkern::SubscribeSignaLog
	void Raumkern::SubscribeSignalLog(const typeSignalRaumkernLog::slot_type &_subscriber)
	{
		signalLog.connect(_subscriber);
	}

	// Raumkern::SubscribeSignalWebServerReady
	void Raumkern::SubscribeSignalWebServerReady(const typeSignalWebServerReady::slot_type &_subscriber)
	{
		if (raumkernServer)
			raumkernServer->SubscribeSignalWebServerReady(_subscriber);
	}

	// Raumkern::SubscribeSignalWebServerShutdown
	void Raumkern::SubscribeSignalWebServerShutdown(const typeSignalWebServerShutdown::slot_type &_subscriber)
	{
		if (raumkernServer)
			raumkernServer->SubscribeSignalWebServerShutdown(_subscriber);
	}

	// Raumkern::SubscribeSignalWebServerStartFailed
	void Raumkern::SubscribeSignalWebServerStartFailed(const typeSignalWebServerStartFailed::slot_type &_subscriber)
	{
		if (raumkernServer)
			raumkernServer->SubscribeSignalWebServerStartFailed(_subscriber);
	}

	// Raumkern::SubscribeSignalMediaServerFound
	void Raumkern::SubscribeSignalMediaServerFound(const typeSignalMediaServerFound::slot_type &_subscriber)
	{
		if (managerList.deviceManager)
			managerList.deviceManager->SubscribeSignalMediaServerFound(_subscriber);
	}

	// Raumkern::SubscribeSignalMediaServerLost
	void Raumkern::SubscribeSignalMediaServerLost(const typeSignalMediaServerLost::slot_type &_subscriber)
	{
		if (managerList.deviceManager)
			managerList.deviceManager->SubscribeSignalMediaServerLost(_subscriber);
	}

}