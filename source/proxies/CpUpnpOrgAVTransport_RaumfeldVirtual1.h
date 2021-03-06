#ifndef HEADER_UPNPORGAVTRANSPORT_RAUMFELDVIRTUAL1CPP
#define HEADER_UPNPORGAVTRANSPORT_RAUMFELDVIRTUAL1CPP

#include <OpenHome/Types.h>
#include <OpenHome/Exception.h>
#include <OpenHome/Functor.h>
#include <OpenHome/Net/Core/FunctorAsync.h>
#include <OpenHome/Net/Core/CpProxy.h>

#include <string>

namespace OpenHome {
namespace Net {

class CpDeviceCpp;
class Action;
class PropertyBinary;
class PropertyBool;
class PropertyInt;
class PropertyString;
class PropertyUint;

/**
 * Proxy for upnp.org:AVTransport_RaumfeldVirtual:1
 * @ingroup Proxies
 */
class CpProxyUpnpOrgAVTransport_RaumfeldVirtual1Cpp : public CpProxy
{
public:
    /**
     * Constructor.
     *
     * Use CpProxy::[Un]Subscribe() to enable/disable querying of state variable
     * and reporting of their changes.
     *
     * @param[in]  aDevice   The device to use
     */
    CpProxyUpnpOrgAVTransport_RaumfeldVirtual1Cpp(CpDeviceCpp& aDevice);
    /**
     * Destructor.
     * If any asynchronous method is in progress, this will block until they complete.
     * [Note that any methods still in progress are likely to complete with an error.]
     * Clients who have called Subscribe() do not need to call Unsubscribe() before
     * calling delete.  An unsubscribe will be triggered automatically when required.
     */
    ~CpProxyUpnpOrgAVTransport_RaumfeldVirtual1Cpp();

    /**
     * Invoke the action synchronously.  Blocks until the action has been processed
     * on the device and sets any output arguments.
     *
     * @param[in]  aInstanceID
     * @param[in]  aCurrentURI
     * @param[in]  aCurrentURIMetaData
     */
    void SyncSetAVTransportURI(uint32_t aInstanceID, const std::string& aCurrentURI, const std::string& aCurrentURIMetaData);
    /**
     * Invoke the action asynchronously.
     * Returns immediately and will run the client-specified callback when the action
     * later completes.  Any output arguments can then be retrieved by calling
     * EndSetAVTransportURI().
     *
     * @param[in] aInstanceID
     * @param[in] aCurrentURI
     * @param[in] aCurrentURIMetaData
     * @param[in] aFunctor   Callback to run when the action completes.
     *                       This is guaranteed to be run but may indicate an error
     */
    void BeginSetAVTransportURI(uint32_t aInstanceID, const std::string& aCurrentURI, const std::string& aCurrentURIMetaData, FunctorAsync& aFunctor);
    /**
     * Retrieve the output arguments from an asynchronously invoked action.
     * This may only be called from the callback set in the above Begin function.
     *
     * @param[in]  aAsync  Argument passed to the callback set in the above Begin function
     */
    void EndSetAVTransportURI(IAsync& aAsync);

    /**
     * Invoke the action synchronously.  Blocks until the action has been processed
     * on the device and sets any output arguments.
     *
     * @param[in]  aInstanceID
     * @param[in]  aCurrentURI
     * @param[in]  aCurrentURIMetaData
     */
    void SyncBendAVTransportURI(uint32_t aInstanceID, const std::string& aCurrentURI, const std::string& aCurrentURIMetaData);
    /**
     * Invoke the action asynchronously.
     * Returns immediately and will run the client-specified callback when the action
     * later completes.  Any output arguments can then be retrieved by calling
     * EndBendAVTransportURI().
     *
     * @param[in] aInstanceID
     * @param[in] aCurrentURI
     * @param[in] aCurrentURIMetaData
     * @param[in] aFunctor   Callback to run when the action completes.
     *                       This is guaranteed to be run but may indicate an error
     */
    void BeginBendAVTransportURI(uint32_t aInstanceID, const std::string& aCurrentURI, const std::string& aCurrentURIMetaData, FunctorAsync& aFunctor);
    /**
     * Retrieve the output arguments from an asynchronously invoked action.
     * This may only be called from the callback set in the above Begin function.
     *
     * @param[in]  aAsync  Argument passed to the callback set in the above Begin function
     */
    void EndBendAVTransportURI(IAsync& aAsync);

    /**
     * Invoke the action synchronously.  Blocks until the action has been processed
     * on the device and sets any output arguments.
     *
     * @param[in]  aResourceURI
     */
    void SyncSetResourceForCurrentStream(const std::string& aResourceURI);
    /**
     * Invoke the action asynchronously.
     * Returns immediately and will run the client-specified callback when the action
     * later completes.  Any output arguments can then be retrieved by calling
     * EndSetResourceForCurrentStream().
     *
     * @param[in] aResourceURI
     * @param[in] aFunctor   Callback to run when the action completes.
     *                       This is guaranteed to be run but may indicate an error
     */
    void BeginSetResourceForCurrentStream(const std::string& aResourceURI, FunctorAsync& aFunctor);
    /**
     * Retrieve the output arguments from an asynchronously invoked action.
     * This may only be called from the callback set in the above Begin function.
     *
     * @param[in]  aAsync  Argument passed to the callback set in the above Begin function
     */
    void EndSetResourceForCurrentStream(IAsync& aAsync);

    /**
     * Invoke the action synchronously.  Blocks until the action has been processed
     * on the device and sets any output arguments.
     *
     * @param[in]  aInstanceID
     * @param[out] aNrTracks
     * @param[out] aMediaDuration
     * @param[out] aCurrentURI
     * @param[out] aCurrentURIMetaData
     * @param[out] aNextURI
     * @param[out] aNextURIMetaData
     * @param[out] aPlayMedium
     * @param[out] aRecordMedium
     * @param[out] aWriteStatus
     */
    void SyncGetMediaInfo(uint32_t aInstanceID, uint32_t& aNrTracks, std::string& aMediaDuration, std::string& aCurrentURI, std::string& aCurrentURIMetaData, std::string& aNextURI, std::string& aNextURIMetaData, std::string& aPlayMedium, std::string& aRecordMedium, std::string& aWriteStatus);
    /**
     * Invoke the action asynchronously.
     * Returns immediately and will run the client-specified callback when the action
     * later completes.  Any output arguments can then be retrieved by calling
     * EndGetMediaInfo().
     *
     * @param[in] aInstanceID
     * @param[in] aFunctor   Callback to run when the action completes.
     *                       This is guaranteed to be run but may indicate an error
     */
    void BeginGetMediaInfo(uint32_t aInstanceID, FunctorAsync& aFunctor);
    /**
     * Retrieve the output arguments from an asynchronously invoked action.
     * This may only be called from the callback set in the above Begin function.
     *
     * @param[in]  aAsync  Argument passed to the callback set in the above Begin function
     * @param[out] aNrTracks
     * @param[out] aMediaDuration
     * @param[out] aCurrentURI
     * @param[out] aCurrentURIMetaData
     * @param[out] aNextURI
     * @param[out] aNextURIMetaData
     * @param[out] aPlayMedium
     * @param[out] aRecordMedium
     * @param[out] aWriteStatus
     */
    void EndGetMediaInfo(IAsync& aAsync, uint32_t& aNrTracks, std::string& aMediaDuration, std::string& aCurrentURI, std::string& aCurrentURIMetaData, std::string& aNextURI, std::string& aNextURIMetaData, std::string& aPlayMedium, std::string& aRecordMedium, std::string& aWriteStatus);

    /**
     * Invoke the action synchronously.  Blocks until the action has been processed
     * on the device and sets any output arguments.
     *
     * @param[in]  aInstanceID
     * @param[out] aCurrentTransportState
     * @param[out] aCurrentTransportStatus
     * @param[out] aCurrentSpeed
     */
    void SyncGetTransportInfo(uint32_t aInstanceID, std::string& aCurrentTransportState, std::string& aCurrentTransportStatus, std::string& aCurrentSpeed);
    /**
     * Invoke the action asynchronously.
     * Returns immediately and will run the client-specified callback when the action
     * later completes.  Any output arguments can then be retrieved by calling
     * EndGetTransportInfo().
     *
     * @param[in] aInstanceID
     * @param[in] aFunctor   Callback to run when the action completes.
     *                       This is guaranteed to be run but may indicate an error
     */
    void BeginGetTransportInfo(uint32_t aInstanceID, FunctorAsync& aFunctor);
    /**
     * Retrieve the output arguments from an asynchronously invoked action.
     * This may only be called from the callback set in the above Begin function.
     *
     * @param[in]  aAsync  Argument passed to the callback set in the above Begin function
     * @param[out] aCurrentTransportState
     * @param[out] aCurrentTransportStatus
     * @param[out] aCurrentSpeed
     */
    void EndGetTransportInfo(IAsync& aAsync, std::string& aCurrentTransportState, std::string& aCurrentTransportStatus, std::string& aCurrentSpeed);

    /**
     * Invoke the action synchronously.  Blocks until the action has been processed
     * on the device and sets any output arguments.
     *
     * @param[in]  aInstanceID
     * @param[out] aTrack
     * @param[out] aTrackDuration
     * @param[out] aTrackMetaData
     * @param[out] aTrackURI
     * @param[out] aRelTime
     * @param[out] aAbsTime
     * @param[out] aRelCount
     * @param[out] aAbsCount
     */
    void SyncGetPositionInfo(uint32_t aInstanceID, uint32_t& aTrack, std::string& aTrackDuration, std::string& aTrackMetaData, std::string& aTrackURI, std::string& aRelTime, std::string& aAbsTime, int32_t& aRelCount, int32_t& aAbsCount);
    /**
     * Invoke the action asynchronously.
     * Returns immediately and will run the client-specified callback when the action
     * later completes.  Any output arguments can then be retrieved by calling
     * EndGetPositionInfo().
     *
     * @param[in] aInstanceID
     * @param[in] aFunctor   Callback to run when the action completes.
     *                       This is guaranteed to be run but may indicate an error
     */
    void BeginGetPositionInfo(uint32_t aInstanceID, FunctorAsync& aFunctor);
    /**
     * Retrieve the output arguments from an asynchronously invoked action.
     * This may only be called from the callback set in the above Begin function.
     *
     * @param[in]  aAsync  Argument passed to the callback set in the above Begin function
     * @param[out] aTrack
     * @param[out] aTrackDuration
     * @param[out] aTrackMetaData
     * @param[out] aTrackURI
     * @param[out] aRelTime
     * @param[out] aAbsTime
     * @param[out] aRelCount
     * @param[out] aAbsCount
     */
    void EndGetPositionInfo(IAsync& aAsync, uint32_t& aTrack, std::string& aTrackDuration, std::string& aTrackMetaData, std::string& aTrackURI, std::string& aRelTime, std::string& aAbsTime, int32_t& aRelCount, int32_t& aAbsCount);

    /**
     * Invoke the action synchronously.  Blocks until the action has been processed
     * on the device and sets any output arguments.
     *
     * @param[in]  aInstanceID
     * @param[out] aPlayMedia
     * @param[out] aRecMedia
     * @param[out] aRecQualityModes
     */
    void SyncGetDeviceCapabilities(uint32_t aInstanceID, std::string& aPlayMedia, std::string& aRecMedia, std::string& aRecQualityModes);
    /**
     * Invoke the action asynchronously.
     * Returns immediately and will run the client-specified callback when the action
     * later completes.  Any output arguments can then be retrieved by calling
     * EndGetDeviceCapabilities().
     *
     * @param[in] aInstanceID
     * @param[in] aFunctor   Callback to run when the action completes.
     *                       This is guaranteed to be run but may indicate an error
     */
    void BeginGetDeviceCapabilities(uint32_t aInstanceID, FunctorAsync& aFunctor);
    /**
     * Retrieve the output arguments from an asynchronously invoked action.
     * This may only be called from the callback set in the above Begin function.
     *
     * @param[in]  aAsync  Argument passed to the callback set in the above Begin function
     * @param[out] aPlayMedia
     * @param[out] aRecMedia
     * @param[out] aRecQualityModes
     */
    void EndGetDeviceCapabilities(IAsync& aAsync, std::string& aPlayMedia, std::string& aRecMedia, std::string& aRecQualityModes);

    /**
     * Invoke the action synchronously.  Blocks until the action has been processed
     * on the device and sets any output arguments.
     *
     * @param[in]  aInstanceID
     * @param[out] aPlayMode
     * @param[out] aRecQualityMode
     */
    void SyncGetTransportSettings(uint32_t aInstanceID, std::string& aPlayMode, std::string& aRecQualityMode);
    /**
     * Invoke the action asynchronously.
     * Returns immediately and will run the client-specified callback when the action
     * later completes.  Any output arguments can then be retrieved by calling
     * EndGetTransportSettings().
     *
     * @param[in] aInstanceID
     * @param[in] aFunctor   Callback to run when the action completes.
     *                       This is guaranteed to be run but may indicate an error
     */
    void BeginGetTransportSettings(uint32_t aInstanceID, FunctorAsync& aFunctor);
    /**
     * Retrieve the output arguments from an asynchronously invoked action.
     * This may only be called from the callback set in the above Begin function.
     *
     * @param[in]  aAsync  Argument passed to the callback set in the above Begin function
     * @param[out] aPlayMode
     * @param[out] aRecQualityMode
     */
    void EndGetTransportSettings(IAsync& aAsync, std::string& aPlayMode, std::string& aRecQualityMode);

    /**
     * Invoke the action synchronously.  Blocks until the action has been processed
     * on the device and sets any output arguments.
     *
     * @param[in]  aInstanceID
     */
    void SyncStop(uint32_t aInstanceID);
    /**
     * Invoke the action asynchronously.
     * Returns immediately and will run the client-specified callback when the action
     * later completes.  Any output arguments can then be retrieved by calling
     * EndStop().
     *
     * @param[in] aInstanceID
     * @param[in] aFunctor   Callback to run when the action completes.
     *                       This is guaranteed to be run but may indicate an error
     */
    void BeginStop(uint32_t aInstanceID, FunctorAsync& aFunctor);
    /**
     * Retrieve the output arguments from an asynchronously invoked action.
     * This may only be called from the callback set in the above Begin function.
     *
     * @param[in]  aAsync  Argument passed to the callback set in the above Begin function
     */
    void EndStop(IAsync& aAsync);

    /**
     * Invoke the action synchronously.  Blocks until the action has been processed
     * on the device and sets any output arguments.
     *
     * @param[in]  aInstanceID
     * @param[in]  aSpeed
     */
    void SyncPlay(uint32_t aInstanceID, const std::string& aSpeed);
    /**
     * Invoke the action asynchronously.
     * Returns immediately and will run the client-specified callback when the action
     * later completes.  Any output arguments can then be retrieved by calling
     * EndPlay().
     *
     * @param[in] aInstanceID
     * @param[in] aSpeed
     * @param[in] aFunctor   Callback to run when the action completes.
     *                       This is guaranteed to be run but may indicate an error
     */
    void BeginPlay(uint32_t aInstanceID, const std::string& aSpeed, FunctorAsync& aFunctor);
    /**
     * Retrieve the output arguments from an asynchronously invoked action.
     * This may only be called from the callback set in the above Begin function.
     *
     * @param[in]  aAsync  Argument passed to the callback set in the above Begin function
     */
    void EndPlay(IAsync& aAsync);

    /**
     * Invoke the action synchronously.  Blocks until the action has been processed
     * on the device and sets any output arguments.
     *
     * @param[in]  aInstanceID
     */
    void SyncPause(uint32_t aInstanceID);
    /**
     * Invoke the action asynchronously.
     * Returns immediately and will run the client-specified callback when the action
     * later completes.  Any output arguments can then be retrieved by calling
     * EndPause().
     *
     * @param[in] aInstanceID
     * @param[in] aFunctor   Callback to run when the action completes.
     *                       This is guaranteed to be run but may indicate an error
     */
    void BeginPause(uint32_t aInstanceID, FunctorAsync& aFunctor);
    /**
     * Retrieve the output arguments from an asynchronously invoked action.
     * This may only be called from the callback set in the above Begin function.
     *
     * @param[in]  aAsync  Argument passed to the callback set in the above Begin function
     */
    void EndPause(IAsync& aAsync);

    /**
     * Invoke the action synchronously.  Blocks until the action has been processed
     * on the device and sets any output arguments.
     *
     * @param[in]  aInstanceID
     * @param[in]  aUnit
     * @param[in]  aTarget
     */
    void SyncSeek(uint32_t aInstanceID, const std::string& aUnit, const std::string& aTarget);
    /**
     * Invoke the action asynchronously.
     * Returns immediately and will run the client-specified callback when the action
     * later completes.  Any output arguments can then be retrieved by calling
     * EndSeek().
     *
     * @param[in] aInstanceID
     * @param[in] aUnit
     * @param[in] aTarget
     * @param[in] aFunctor   Callback to run when the action completes.
     *                       This is guaranteed to be run but may indicate an error
     */
    void BeginSeek(uint32_t aInstanceID, const std::string& aUnit, const std::string& aTarget, FunctorAsync& aFunctor);
    /**
     * Retrieve the output arguments from an asynchronously invoked action.
     * This may only be called from the callback set in the above Begin function.
     *
     * @param[in]  aAsync  Argument passed to the callback set in the above Begin function
     */
    void EndSeek(IAsync& aAsync);

    /**
     * Invoke the action synchronously.  Blocks until the action has been processed
     * on the device and sets any output arguments.
     *
     * @param[in]  aInstanceID
     */
    void SyncNext(uint32_t aInstanceID);
    /**
     * Invoke the action asynchronously.
     * Returns immediately and will run the client-specified callback when the action
     * later completes.  Any output arguments can then be retrieved by calling
     * EndNext().
     *
     * @param[in] aInstanceID
     * @param[in] aFunctor   Callback to run when the action completes.
     *                       This is guaranteed to be run but may indicate an error
     */
    void BeginNext(uint32_t aInstanceID, FunctorAsync& aFunctor);
    /**
     * Retrieve the output arguments from an asynchronously invoked action.
     * This may only be called from the callback set in the above Begin function.
     *
     * @param[in]  aAsync  Argument passed to the callback set in the above Begin function
     */
    void EndNext(IAsync& aAsync);

    /**
     * Invoke the action synchronously.  Blocks until the action has been processed
     * on the device and sets any output arguments.
     *
     * @param[in]  aInstanceID
     */
    void SyncPrevious(uint32_t aInstanceID);
    /**
     * Invoke the action asynchronously.
     * Returns immediately and will run the client-specified callback when the action
     * later completes.  Any output arguments can then be retrieved by calling
     * EndPrevious().
     *
     * @param[in] aInstanceID
     * @param[in] aFunctor   Callback to run when the action completes.
     *                       This is guaranteed to be run but may indicate an error
     */
    void BeginPrevious(uint32_t aInstanceID, FunctorAsync& aFunctor);
    /**
     * Retrieve the output arguments from an asynchronously invoked action.
     * This may only be called from the callback set in the above Begin function.
     *
     * @param[in]  aAsync  Argument passed to the callback set in the above Begin function
     */
    void EndPrevious(IAsync& aAsync);

    /**
     * Invoke the action synchronously.  Blocks until the action has been processed
     * on the device and sets any output arguments.
     *
     * @param[in]  aInstanceID
     * @param[in]  aNewPlayMode
     */
    void SyncSetPlayMode(uint32_t aInstanceID, const std::string& aNewPlayMode);
    /**
     * Invoke the action asynchronously.
     * Returns immediately and will run the client-specified callback when the action
     * later completes.  Any output arguments can then be retrieved by calling
     * EndSetPlayMode().
     *
     * @param[in] aInstanceID
     * @param[in] aNewPlayMode
     * @param[in] aFunctor   Callback to run when the action completes.
     *                       This is guaranteed to be run but may indicate an error
     */
    void BeginSetPlayMode(uint32_t aInstanceID, const std::string& aNewPlayMode, FunctorAsync& aFunctor);
    /**
     * Retrieve the output arguments from an asynchronously invoked action.
     * This may only be called from the callback set in the above Begin function.
     *
     * @param[in]  aAsync  Argument passed to the callback set in the above Begin function
     */
    void EndSetPlayMode(IAsync& aAsync);

    /**
     * Invoke the action synchronously.  Blocks until the action has been processed
     * on the device and sets any output arguments.
     *
     * @param[out] aCurrentContentType
     * @param[out] aCurrentBitrate
     */
    void SyncGetStreamProperties(std::string& aCurrentContentType, uint32_t& aCurrentBitrate);
    /**
     * Invoke the action asynchronously.
     * Returns immediately and will run the client-specified callback when the action
     * later completes.  Any output arguments can then be retrieved by calling
     * EndGetStreamProperties().
     *
     * @param[in] aFunctor   Callback to run when the action completes.
     *                       This is guaranteed to be run but may indicate an error
     */
    void BeginGetStreamProperties(FunctorAsync& aFunctor);
    /**
     * Retrieve the output arguments from an asynchronously invoked action.
     * This may only be called from the callback set in the above Begin function.
     *
     * @param[in]  aAsync  Argument passed to the callback set in the above Begin function
     * @param[out] aCurrentContentType
     * @param[out] aCurrentBitrate
     */
    void EndGetStreamProperties(IAsync& aAsync, std::string& aCurrentContentType, uint32_t& aCurrentBitrate);

    /**
     * Invoke the action synchronously.  Blocks until the action has been processed
     * on the device and sets any output arguments.
     *
     * @param[in]  aInstanceID
     * @param[out] aActions
     */
    void SyncGetCurrentTransportActions(uint32_t aInstanceID, std::string& aActions);
    /**
     * Invoke the action asynchronously.
     * Returns immediately and will run the client-specified callback when the action
     * later completes.  Any output arguments can then be retrieved by calling
     * EndGetCurrentTransportActions().
     *
     * @param[in] aInstanceID
     * @param[in] aFunctor   Callback to run when the action completes.
     *                       This is guaranteed to be run but may indicate an error
     */
    void BeginGetCurrentTransportActions(uint32_t aInstanceID, FunctorAsync& aFunctor);
    /**
     * Retrieve the output arguments from an asynchronously invoked action.
     * This may only be called from the callback set in the above Begin function.
     *
     * @param[in]  aAsync  Argument passed to the callback set in the above Begin function
     * @param[out] aActions
     */
    void EndGetCurrentTransportActions(IAsync& aAsync, std::string& aActions);

    /**
     * Set a callback to be run when the LastChange state variable changes.
     *
     * Callbacks may be run in different threads but callbacks for a
     * CpProxyUpnpOrgAVTransport_RaumfeldVirtual1Cpp instance will not overlap.
     *
     * @param[in]  aFunctor  The callback to run when the state variable changes
     */
    void SetPropertyLastChangeChanged(Functor& aFunctor);

    /**
     * Query the value of the LastChange property.
     *
     * This function is threadsafe and can only be called if Subscribe() has been
     * called and a first eventing callback received more recently than any call
     * to Unsubscribe().
     *
     * @param[out] aLastChange
     */
    void PropertyLastChange(std::string& aLastChange) const;
private:
    void LastChangePropertyChanged();
private:
    Action* iActionSetAVTransportURI;
    Action* iActionBendAVTransportURI;
    Action* iActionSetResourceForCurrentStream;
    Action* iActionGetMediaInfo;
    Action* iActionGetTransportInfo;
    Action* iActionGetPositionInfo;
    Action* iActionGetDeviceCapabilities;
    Action* iActionGetTransportSettings;
    Action* iActionStop;
    Action* iActionPlay;
    Action* iActionPause;
    Action* iActionSeek;
    Action* iActionNext;
    Action* iActionPrevious;
    Action* iActionSetPlayMode;
    Action* iActionGetStreamProperties;
    Action* iActionGetCurrentTransportActions;
    PropertyString* iLastChange;
    Functor iLastChangeChanged;
};

} // namespace Net
} // namespace OpenHome

#endif // HEADER_UPNPORGAVTRANSPORT_RAUMFELDVIRTUAL1CPP

