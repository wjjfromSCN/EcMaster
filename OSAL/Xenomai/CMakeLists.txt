cmake_minimum_required(VERSION 2.8.12)
project(XenomaiOSAL)
include_directories(
    /usr/xenomai/include/
    /usr/xenomai/include/cobalt
    /usr/xenomai/include/alchemy
    /usr/xenomai/include/trank
)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -DTEST   -ffunction-sections  -ffunction-sections  -D_GNU_SOURCE -D_REENTRANT -D__XENO__")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -DTEST  -Wl,-gc-sections -Wl,--no-as-needed -Wl,@/usr/xenomai/lib/cobalt.wrappers -Wl,@/usr/xenomai/lib/modechk.wrappers  /usr/xenomai/lib/xenomai/bootstrap.o -Wl,--wrap=main -Wl,--dynamic-list=/usr/xenomai/lib/dynlist.ld -Wl,-rpath /usr/xenomai/lib" )
aux_source_directory(. Xenomai_LIST)
add_library(Xenomai  ${Xenomai_LIST}  )
target_link_libraries(Xenomai      pthread  /usr/xenomai/lib/libalchemy.so /usr/xenomai/lib/libanalogy.so /usr/xenomai/lib/libcobalt.so /usr/xenomai/lib/libcopperplate.so /usr/xenomai/lib/libmodechk.so /usr/xenomai/lib/libsmokey.so)
