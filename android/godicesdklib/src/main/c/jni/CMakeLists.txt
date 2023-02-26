cmake_minimum_required(VERSION 3.4.1)

add_library(godicesdklib SHARED
			jni_def.c
			../../../../../../common/godiceapi.c)

target_include_directories(godicesdklib PRIVATE "../../../../../../common")
target_link_libraries(godicesdklib android log)
