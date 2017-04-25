cmake_minimum_required(VERSION 3.3)
project(MP)

set(IA_WINDOWS 0)
set(IA_LINUX 0)
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
	set (MP_LINUX 1)
elseif (CMAKE_SYSTEM_NAME  MATCHES "Windows")
	set (MP_WINDOWS 1)
endif (CMAKE_SYSTEM_NAME MATCHES "Linux")

if (MP_WINDOWS)
    source_group(src REGULAR_EXPRESSION "src/.+\.(h|inc|cc)")
endif (MP_WINDOWS)
include_directories(inc thrift/gen-cpp odb proto)
aux_source_directory(src SRC_LIST)
aux_source_directory(thrift/gen-cpp THRIFT)
aux_source_directory(odb ODB)
aux_source_directory(proto PROTO)

message(STATUS "This is Source dir: " ${PROJECT_SOURCE_DIR})
message(STATUS "This is Binary dir: " ${PROJECT_BINARY_DIR})
message(STATUS "SRC_LIST: " ${SRC_LIST})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wl,-rpath=/opt/libjpeg-turbo/lib64 -g")
link_directories(lib/tj lib/lpr lib/boost lib/protobuf lib/gflags lib)
add_executable(mserver ${SRC_LIST} ${THRIFT} ${ODB} ${PROTO})
target_link_libraries(mserver 
 				pthread
				glog
 				boost_serialization
				boost_filesystem
				boost_system
				boost_thread
				turbojpeg
				dsplpr
				thrift
				odb
				odb-sqlite
				protobuf
				gflags
				profiler
				unwind
				)
