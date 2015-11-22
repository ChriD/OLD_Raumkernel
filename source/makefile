# Makefile template for shared library
 
INCPATH = -I/usr/packages/boost_1_57_0 -I/usr/packages/cpp-netlib-0.11.1-final -I/usr/packages/rapidxml_1_0_0 -I/usr/packages/ohNet-Linux-x86-Release/include/ohnet

CC = g++-4.9
CXX = g++-4.9
CFLAGS = -std=c++0x -fPIC -Wall -Wextra -O2 -c -pthread
CXXFLAGS = -std=c++0x -fPIC -Wall -Wextra -O2 -c -pthread

LDFLAGS = -shared -pthread -static-libgcc -static-libstdc++

LIBSDEF = /usr/packages/libs/libohNetCore.a /usr/packages/libs/libohNetDevices.a /usr/packages/libs/libohNetProxies.a /usr/packages/libs/libcppnetlib-uri.a /usr/packages/libs/libboost_system.a /usr/packages/libs/libboost_thread.a /usr/packages/libs/libboost_signals.a /usr/packages/libs/libboost_date_time.a /usr/packages/libs/libboost_regex.a 

RM = rm -f 
TARGET_LIB = /usr/packages/libs/raumkern.so
 
SRCS = Raumkern.cpp DeviceManager.cpp Global.cpp Logger.cpp Manager.cpp RaumkernObject.cpp RaumkernSignal.cpp RaumkernUPNP.cpp Renderer.cpp ZoneManager.cpp MediaServer.cpp ManagerList.cpp proxies/CpUpnpOrgAVTransport_RaumfeldVirtual1Std.cpp proxies/CpUpnpOrgConnectionManager_RaumfeldVirtual1Std.cpp proxies/CpUpnpOrgRenderingControl_RaumfeldVirtual1Std.cpp proxies/CpUpnpOrgContentDirectory_Raumfeld1Std.cpp WebServer.cpp urlcpp/Path.cpp urlcpp/Url.cpp WebServerResponseHandler.cpp MediaItem.cpp MediaBrowser.cpp ContentManager.cpp proxies/CpUpnpOrgContentDirectory1.cpp

OBJS = $(SRCS:.cpp=.o)

.PHONY: all
all: ${TARGET_LIB}
 
$(TARGET_LIB): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^ $(LIBSDEF)
 
$(SRCS:.cpp=.d):%.d:%.cpp
	$(CC) $(CFLAGS) -MM $< >$@
 
include $(SRCS:.cpp=.d)
 

.PHONY: clean
clean:
	-${RM} ${TARGET_LIB} ${OBJS} $(SRCS:.cpp=.d) 