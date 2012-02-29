cmake_minimum_required(VERSION 2.8)
project(oocl)

add_subdirectory(demos)

set(Headers include/DirectConNetwork.h include/ExplicitMessages.h include/Log.h include/Message.h include/MessageBroker.h include/MessageListener.h include/Peer.h include/Peer2PeerNetwork.h include/ServerSocket.h include/Socket.h include/SocketStub.h include/Thread.h)
set(Sources src/DirectConNetwork.cpp src/ExplicitMessages.cpp src/Log.cpp src/Message.cpp src/MessageBroker.cpp src/MessageListener.cpp src/Peer.cpp src/Peer2PeerNetwork.cpp src/ServerSocket.cpp src/Socket.cpp src/SocketStub.cpp src/Thread.cpp)

include_directories (include) 

add_definitions ( "-D OOCL_EXPORT" )

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${oocl_SOURCE_DIR}/bin)

add_library (oocl SHARED ${Headers} ${Sources})

if (MSVC)
   target_link_libraries (oocl Ws2_32)
endif (MSVC)