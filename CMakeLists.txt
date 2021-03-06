cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

project(EasyRPG_Player CXX C)

# liblcf
# - ${LIBLCF_PATH}

set(DEFAULT_LIBLCF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/liblcf")
if(NOT LIBLCF_PATH)
  set(LIBLCF_PATH "${DEFAULT_LIBLCF_PATH}")
endif()
find_package(Git REQUIRED)
if(NOT EXISTS ${LIBLCF_PATH})
  execute_process(COMMAND ${GIT_EXECUTABLE} clone "--depth=1"
    "https://github.com/EasyRPG/liblcf.git"
    "${LIBLCF_PATH}")
endif()
add_subdirectory(
  ${LIBLCF_PATH}/builds/cmake
  ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/liblcf)
link_directories(${ICU_LIBRARY_DIRS})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib)
#link_directories(${LIBRARY_OUTPUT_PATH})
#cmake_policy(CMP0015 NEW)

list(APPEND CMAKE_MODULE_PATH
	"${CMAKE_CURRENT_SOURCE_DIR}/lib/liblcf/builds/cmake/Modules"
	"${CMAKE_CURRENT_SOURCE_DIR}/builds/cmake/Modules")

add_definitions(
  ${PNG_DEFINITIONS}

  -D USE_SDL=1)

if(CMAKE_GENERATOR MATCHES "Makefile")
  add_definitions(-fno-rtti)
endif()

# wine checking
find_program(WINE_EXECUTABLE wine)
if(EXISTS ${WINE_EXECUTABLE})
  message(STATUS "wine registry enabled")
  add_definitions(-DHAVE_WINE=1)
endif()

# shinonome font
set(SHINONOME_PATH "${CMAKE_CURRENT_SOURCE_DIR}/resources/shinonome")
set(SHINONOME_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SHINONOME_SRCS "${SHINONOME_SRC_PATH}/shinonome_gothic.cpp"
  "${SHINONOME_SRC_PATH}/shinonome_mincho.cpp"
  "${SHINONOME_SRC_PATH}/shinonome.h")
find_package(Ruby)
if(RUBY_FOUND)
  set(SHINONOME_GENERATOR "${SHINONOME_PATH}/generate_cxx_font.rb")
  add_custom_command(
    OUTPUT ${SHINONOME_SRCS}
    COMMAND "${RUBY_EXECUTABLE}" ARGS "${SHINONOME_GENERATOR}"
    DEPENDS "${SHINONOME_GENERATOR}"
    WORKING_DIRECTORY "${SHINONOME_PATH}")
endif()

# endianess check
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  include(TestBigEndian)
  test_big_endian(WORDS_BIGENDIAN)
  if(WORDS_BIGENDIAN)
    add_definitions(-D WORDS_BIGENDIAN=1)
  endif()
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
  add_definitions(-D _DEBUG=1)
endif()

# mpg123
find_package(libmpg123)
if (libmpg123_FOUND)
  include_directories(${MPG123_INCLUDE_DIR})
  list(APPEND EASYRPG_PLAYER_LIBRARIES ${MPG123_LIBRARY})
  add_definitions(-DHAVE_MPG123)
endif()

# SDL_mixer
find_package(SDL2_mixer)
if(SDL2_MIXER_FOUND)
  include_directories(${SDL2_MIXER_INCLUDE_DIR})
  list(APPEND EASYRPG_PLAYER_LIBRARIES ${SDL2_MIXER_LIBRARY})
  add_definitions(-D HAVE_SDL_MIXER=1)
else()
  add_definitions(-DNO_SDL_MIXER)
endif()

# OpenAL
find_package(OpenAL)
if(OPENAL_FOUND)
  find_package(SndFile REQUIRED)
  find_package(FluidSynth REQUIRED)
  include_directories(${OPENAL_INCLUDE_DIR} ${SNDFILE_INCLUDE_DIR} ${FLUIDSYNTH_INCLUDE_DIR})
  list(APPEND EASYRPG_PLAYER_LIBRARIES ${OPENAL_LIBRARY} ${SNDFILE_LIBRARIES} ${FLUIDSYNTH_LIBRARY})
  add_definitions(-D HAVE_OPENAL=1)
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/liblcf/src")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/liblcf/src/generated")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")

foreach(i Expat Freetype Harfbuzz Pixman ZLIB PNG SDL2 Iconv)
  find_package(${i} REQUIRED)

  string(TOUPPER ${i} i)

  include_directories(${${i}_INCLUDE_DIR} ${${i}_INCLUDE_DIRS})
  list(APPEND EASYRPG_PLAYER_LIBRARIES ${${i}_LIBRARY}  ${${i}_LIBRARIES})
endforeach()

# library
set(EASYRPG_PLAYER_LIBRARIES_ALL
  ${EASYRPG_PLAYER_LIBRARIES} "${PROJECT_NAME}_Static" liblcf)

# entry point
set(MAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

# static library
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRCS)
list(REMOVE_ITEM SRCS ${MAIN_FILE})
list(APPEND SRCS ${SHINONOME_SRCS})
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
  list(APPEND SRCS "${CMAKE_CURRENT_SOURCE_DIR}/resources/player.rc")
endif()
add_library(${PROJECT_NAME}_Static STATIC ${SRCS})

# executable
add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${MAIN_FILE})
add_executable(${PROJECT_NAME}_com ${MAIN_FILE})
foreach(i ${PROJECT_NAME} ${PROJECT_NAME}_com)
  target_link_libraries(${i} ${EASYRPG_PLAYER_LIBRARIES_ALL})
  add_dependencies(${i} liblcf ${PROJECT_NAME}_Static)
endforeach()
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME} DESTINATION bin)

# manpage
set(MAN_NAME easyrpg-player.6)
find_program(A2X_EXECUTABLE NAMES a2x a2x.py)
if(NOT A2X_EXECUTABLE STREQUAL "A2X_EXECUTABLE-NOTFOUND")
  add_custom_command(OUTPUT resources/${MAN_NAME}
    COMMAND ${A2X_EXECUTABLE} -a player_version="0.4.1" -f manpage -D ${CMAKE_CURRENT_BINARY_DIR}/resources ${CMAKE_CURRENT_SOURCE_DIR}/resources/${MAN_NAME}.adoc
    DEPENDS resources/${MAN_NAME}.adoc
    COMMENT "(Re-)building manpage ${MAN_NAME}"
    VERBATIM)
  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_custom_target(man ALL DEPENDS resources/${MAN_NAME})
    install(FILES resources/${MAN_NAME} DESTINATION share/man/man6)
  else()
    add_custom_target(man DEPENDS resources/${MAN_NAME})
  endif()
else()
  # no a2x, distribution archive?
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources/${MAN_NAME})
    install(FILES resources/${MAN_NAME} DESTINATION share/man/man6)
  endif()
endif()

# CPack
set(CPACK_GENERATOR "ZIP" "TGZ")
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  list(APPEND CPACK_GENERATOR "NSIS")
endif()

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION "0.4.1")
set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/player.ico")
set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_SOURCE_DIR}/README)

include(CPack)

# Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
  # fake autotools variables
  set(DX_DOCDIR ${CMAKE_CURRENT_BINARY_DIR}/doc)
  set(PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})
  set(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
  configure_file(resources/Doxyfile.in resources/Doxyfile @ONLY)
  unset(DX_DOCDIR)
  unset(PACKAGE_VERSION)
  unset(srcdir)

  add_custom_target(player_doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/resources/Doxyfile
    DEPENDS ${SRCS} ${MAIN_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources
    COMMENT "Generating API documentation with Doxygen" VERBATIM)
  add_custom_target(doc)
  add_dependencies(doc player_doc liblcf_doc)
endif()

# test
enable_testing()

set(TEST_GAME_REPOSITORY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/TestGame")
if(NOT EXISTS ${TEST_GAME_REPOSITORY_PATH})
  execute_process(COMMAND ${GIT_EXECUTABLE} clone
    "https://github.com/EasyRPG/TestGame.git"
    "${TEST_GAME_REPOSITORY_PATH}")
endif()
set(TEST_GAME_PATH "${TEST_GAME_REPOSITORY_PATH}/TestGame-2000")

set(TEST_ENVS "RPG_TEST_GAME_PATH=${TEST_GAME_PATH}")
if(EXISTS $ENV{RPG_RTP_PATH})
  get_filename_component(RPG_RTP_PATH "$ENV{RPG_RTP_PATH}" ABSOLUTE)
  message(STATUS "using rtp path ${RPG_RTP_PATH}")
  list(APPEND TEST_ENVS "RPG_RTP_PATH=${RPG_RTP_PATH}")
endif()

file(GLOB TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)
foreach(i ${TEST_FILES})
  get_filename_component(name "${i}" NAME_WE)
  add_executable(test_${name} ${i})
  target_link_libraries(test_${name} ${EASYRPG_PLAYER_LIBRARIES_ALL})
  add_dependencies(test_${name} ${PROJECT_NAME}_Static)

  add_test(
    NAME test_${name} WORKING_DIRECTORY ${TEST_GAME_PATH}
    COMMAND ${EXECUTABLE_OUTPUT_PATH}/test_${name})
  set_tests_properties(test_${name}
    PROPERTIES ENVIRONMENT "${TEST_ENVS}")
endforeach()
