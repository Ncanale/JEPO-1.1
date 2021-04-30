################################################################################
#   CMakeLists.txt for jepo of JEDI.                                           #
#                                                                              #
#                      - 20. Nov. 2017. Hoyong Jeong (hyjeong@hep.korea.ac.kr) #
################################################################################

#------------------------------------------------------------------------------#
#   Setup the project                                                          #
#------------------------------------------------------------------------------#
cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
project(JEPO)

#------------------------------------------------------------------------------#
#   Find Geant4 package, activating all available UI and Vis drivers by        #
# default. You can set WITH_GEANT4_UIVIS to OFF via the command line or        #
# ccmake/cmake-gui to build a batch mode only executable.                      #
#------------------------------------------------------------------------------#
option(WITH_GEANT4_UIVIS "Build jepo with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
	find_package(Geant4 REQUIRED ui_all vis_all)
else()
	find_package(Geant4 REQUIRED)
endif()

#------------------------------------------------------------------------------#
#   Find ROOT package.                                                         #
#------------------------------------------------------------------------------#
find_package(ROOT REQUIRED)

#------------------------------------------------------------------------------#
#   Setup Geant4 include directories and compile definitions                   #
#   Setup include directory for this project                                   #
#   Setup ROOT include directories and compile definitions.                    #
#------------------------------------------------------------------------------#
include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include)
include(${ROOT_USE_FILE})

#------------------------------------------------------------------------------#
#   Locate sources and headers for this project                                #
#   NB: headers are included so they will show up in IDEs                      #
#------------------------------------------------------------------------------#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#------------------------------------------------------------------------------#
#   Set everything needed for the root dictonary and create the dictionary     #
#------------------------------------------------------------------------------#
#set(Analysis_HEADERS ${PROJECT_SOURCE_DIR}/include/JediClasses.hh)
#set(Analysis_LINKDEF ${PROJECT_SOURCE_DIR}/include/AnalysisLinkDef.hh)

#ROOT_GENERATE_DICTIONARY(libAnalysis ${Analysis_HEADERS} LINKDEF ${Analysis_LINKDEF})

#add_library(Analysis SHARED libAnalysis.cxx)

#target_link_libraries(Analysis ${Geant4_LIBRARIES} ${ROOT_LIBRARIES} -ldl)

#------------------------------------------------------------------------------#
#   Add the executable, and link it to the Geant4 libraries                    #
#------------------------------------------------------------------------------#
add_executable(jepo jepo.cc ${sources} ${headers})

target_link_libraries(jepo ${Geant4_LIBRARIES} ${ROOT_LIBRARIES})# Analysis)

#------------------------------------------------------------------------------#
#   Copy all scripts to the build directory, i.e. the directory in which we    #
# build jepo. This is so that we can run the executable directly       		   #
# because it relies on these scripts being in the current working directory.   #
#------------------------------------------------------------------------------#
set(JEPO_SCRIPTS
	README.md
	init_vis.mac
	vis.mac
	config.cfg
	1M_event.mac
	10k_event.mac
	100k_event.mac
	n_event.mac
)

foreach(_script ${JEPO_SCRIPTS})
	configure_file(
		${PROJECT_SOURCE_DIR}/${_script}
		${PROJECT_BINARY_DIR}/${_script}
		COPYONLY
	)
endforeach()

#------------------------------------------------------------------------------#
#   For internal Geant4 use - but has no effect if you build this jepo #
# standalone                                                                   #
#------------------------------------------------------------------------------#
add_custom_target(JEPO DEPENDS jepo)

#------------------------------------------------------------------------------#
#   Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX       #
#------------------------------------------------------------------------------#
#install(TARGETS jepo Analysis DESTINATION bin)
install(TARGETS jepo DESTINATION bin)
install(FILES "${PROJECT_BINARY_DIR}/libAnalysis_rdict.pcm" DESTINATION .)

