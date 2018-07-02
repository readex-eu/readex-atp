cmake_minimum_required (VERSION 3.1)
project (ATP)
#version number.
set (ATP_VERSION_MAJOR 1)
set (ATP_VERSION_MINOR 0)
set (ATP_VERSION_PATCH 0)

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE ATP_GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

include(ExternalProject)

#if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
#    message(STATUS "Setting build type to 'Debug' as none was specified.")
 #    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
 #
    # Set the possible values of build type for cmake-gui
#    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
#        "MinSizeRel" "RelWithDebInfo")
#endif()



set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)
set(LUA_SCRIPTS_DIR ${CMAKE_SOURCE_DIR}/src/atpserver/lua)
set(OC_SOURCE_DIR ${CMAKE_SOURCE_DIR}/share/the-omega-project)
set(OC_EXECUTABLE ${OC_SOURCE_DIR}/src/OmegaCalculator/omega_calc/obj/oc)


set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")


#Add custom compilation flags
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")


#
# READEX RRL is required for the build

find_package(RRL CONFIG REQUIRED)
if(RRL_FOUND)
    message(STATUS "READEX RRL library found")
    MACRO(target_link_rrl tgt)
         include_directories(${RRL_INCLUDE_DIRS})
         target_link_libraries(${tgt} ${RRL_LIBRARIES})
    ENDMACRO()
    message (STATUS "RRL_INCLUDE_DIRS = ${RRL_INCLUDE_DIRS}")
    message (STATUS "RRL_LIBRARIES = ${RRL_LIBRARIES}")
else()
    message(WARNING "READEX RRL library not found")
endif()



#
# lua is required for the build
find_package(Lua REQUIRED)
if(LUA_FOUND)	 
     message(STATUS "Note: recommanded LUA versions are => 5.1")

     MACRO(target_link_lua tgt)
         include_directories(${LUA_INCLUDE_DIR})
         target_link_libraries(${tgt} ${LUA_LIBRARIES})
     ENDMACRO()

else()
     message(SEND_ERROR "LUA not found")
endif(LUA_FOUND)



#
# check that the LUA scripts folder exists and contains the scripts and the OC binary
if(EXISTS "${LUA_SCRIPTS_DIR}")
    message(STATUS "LUA scripts directory found")
    set(LUA_SCRIPTS "${LUA_SCRIPTS_DIR}/constraint_parser.lua"  
                    "${LUA_SCRIPTS_DIR}/lua_codex.lua"   
                    "${LUA_SCRIPTS_DIR}/oc_query.lua"
                    "${LUA_SCRIPTS_DIR}/parse_oc_code.lua"
                    "${LUA_SCRIPTS_DIR}/oc")
    
    foreach(lua_script ${LUA_SCRIPTS})
        if(NOT EXISTS ${lua_script})
            message(WARNING "LUA script <${lua_script}> not found")
        endif()
    endforeach()  
else()
    message(SEND_ERROR"LUA scripts directory not found")
endif()



#
# copy the LUA folder inside the executable folder
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/lib")
   file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
endif()
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/bin")
   file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
endif()
if(NOT EXISTS "${EXECUTABLE_OUTPUT_PATH}/lua")
   file(MAKE_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}/lua")
endif()

configure_file("${LUA_SCRIPTS_DIR}/constraint_parser.lua"  "${EXECUTABLE_OUTPUT_PATH}/lua")
configure_file("${LUA_SCRIPTS_DIR}/lua_codex.lua"          "${EXECUTABLE_OUTPUT_PATH}/lua")
configure_file("${LUA_SCRIPTS_DIR}/oc_query.lua"           "${EXECUTABLE_OUTPUT_PATH}/lua")
configure_file("${LUA_SCRIPTS_DIR}/parse_oc_code.lua"      "${EXECUTABLE_OUTPUT_PATH}/lua")
configure_file("${LUA_SCRIPTS_DIR}/oc"                     "${EXECUTABLE_OUTPUT_PATH}/lua")

#
# Build the OC calculator
if(NOT EXISTS "${OC_SOURCE_DIR}")
   message(SEND_ERROR "Omega Calculator source dir not found")
else()
  message(STATUS "Prepraring build command for Omega Claculator (OC)")
  ExternalProject_Add(OmegaCalculator
		      PREFIX ${OC_SOURCE_DIR}
		      GIT_REPOSITORY https://github.com/davewathaverford/the-omega-project.git
  		      CONFIGURE_COMMAND ""
		      BUILD_IN_SOURCE 1
		      BUILD_COMMAND make clean_all && make oc
		      INSTALL_COMMAND ""
		      UPDATE_COMMAND "")

endif()

                    



#
# copy the atpenv.sh file inside the bin directory and update its content
configure_file(atpenv.sh ${EXECUTABLE_OUTPUT_PATH})

configure_file(include/atpconfig.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/atpconfig.h)
#
#
include_directories("include")

set(ATP_INCLUDES "include/ATP_registry_protocol.h"
                 "include/atpcommon.h"
                 "include/atpdesc.h"
                 "include/atplib.h"
                 "include/atpquery.h"
                 "include/atpreport.h"
                 "include/atpserver.h"
                 "include/json.h"
                 "include/atplib.inc"
		 "include/atpconfig.h")

set(ATPLIB_SOURCES "src/atplib/atplib.c" 
                   "src/atplib/atplibF.c" 
                   "src/common/atpcommon.c" 
                   "src/common/atpreport.c"
                   "src/common/json.c" 
                   "src/common/atpdesc.c")

set(ATPSERVER_SOURCES "src/atpserver/atpserver.c" 
                      "src/atpserver/atpquery.c" 
                      "src/atpserver/main.c" 
                      "src/common/json.c" 
                      "src/common/atpcommon.c" 
                      "src/common/atpreport.c"
                      "src/common/atpdesc.c")


#
# find Doxygen and build documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)

add_library(atp SHARED ${ATPLIB_SOURCES})
add_executable(atpserver ${ATPSERVER_SOURCES})

target_compile_options(atp PUBLIC -g)
target_compile_options(atpserver PUBLIC -g)

target_link_rrl(atp)
target_link_rrl(atpserver)
target_link_lua(atpserver)

if(ATP_BUILD_TESTS)
	message(STATUS "Building ATP tests enabled")
	add_executable(atptest "test/test.c")
	add_executable(atplib_test "test/atplib_test.c")
	add_executable(atpserver_test "test/atpserver_test.c")
	
	target_link_libraries(atptest atp)
	target_link_libraries(atplib_test atp)
	target_link_libraries(atpserver_test atp)

	install(TARGETS atptest DESTINATION test)
	install(TARGETS atplib_test DESTINATION test)
	install(TARGETS atpserver_test DESTINATION test)
endif(ATP_BUILD_TESTS)

install(TARGETS atp DESTINATION lib)
install(FILES ${ATP_INCLUDES} DESTINATION include)
install(FILES ${LUA_SCRIPTS}  DESTINATION bin/lua)
install(FILES ${OC_EXECUTABLE} DESTINATION bin/lua PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
install(TARGETS atpserver DESTINATION bin)



