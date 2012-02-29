project (Peer2PeerTest)

include_directories (../../include) 
link_directories (../../lib) 

set(Headers ChatMessage.h IntroductionMessage.h)
set(Sources Peer2PeerTest.cpp ChatMessage.cpp IntroductionMessage.cpp)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${oocl_SOURCE_DIR}/bin)

add_executable (Peer2PeerTest ${Headers} ${Sources})

target_link_libraries (Peer2PeerTest oocl) 