cmake_minimum_required(VERSION 2.8)
project(oocl)

add_subdirectory(demos)

set(Headers include/DirectConNetwork.h include/ExplicitMessages.h include/Log.h include/Message.h include/MessageBroker.h include/MessageListener.h include/Peer.h include/Peer2PeerNetwork.h include/ServerSocket.h include/Socket.h include/BerkeleySocket.h include/SecureSocket.h include/SocketStub.h include/Thread.h include/HttpClient.h include/oocl_import_export.h)
set(Sources src/DirectConNetwork.cpp src/ExplicitMessages.cpp src/Log.cpp src/Message.cpp src/MessageBroker.cpp src/MessageListener.cpp src/Peer.cpp src/Peer2PeerNetwork.cpp src/ServerSocket.cpp src/Socket.cpp src/BerkeleySocket.cpp src/SecureSocket.cpp src/SocketStub.cpp src/Thread.cpp src/HttpClient.cpp)

include_directories (include) 

find_package (Threads)
find_package (Curses)
find_package (OpenSSL)

add_definitions ( "-D OOCL_EXPORT -D OPENSSL" )

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${oocl_SOURCE_DIR}/bin)

add_library (oocl SHARED ${Headers} ${Sources})

if (MSVC)
   target_link_libraries (oocl Ws2_32 ${OPENSSL_LIB})
else (MSVC)
   target_link_libraries (oocl ${CMAKE_THREAD_LIBS_INIT} ${CURSES_LIBRARY} ${OPENSSL_LIB})
endif (MSVC)