project(obsplay)

set(CMAKE_CXX_STANDARD 17)

# set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*)
file(GLOB obsplay_UI_FILES ${PROJECT_SOURCE_DIR}/ui/*.ui)

find_qt(COMPONENTS Widgets Core)
qt5_wrap_ui(obsplay_UI ${obsplay_UI_FILES})

message(CMAKE_CURRENT_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}")

add_library(obsplay MODULE ${SRC_FILES} ${obsplay_UI})

if (UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    list(APPEND ADDITIONAL_LIBS ${X11_LIBRARIES})
endif()

target_include_directories(obsplay PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(obsplay ${frontend-tools_PLATFORM_LIBS} libobs obs-frontend-api Qt::Core Qt::Widgets ${ADDITIONAL_LIBS})
set_target_properties(obsplay PROPERTIES FOLDER "plugins/obsplay")
setup_plugin_target(obsplay data)
