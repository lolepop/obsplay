project(obsplay)

set(CMAKE_CXX_STANDARD 17)

# set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*)
file(GLOB obsplay_UI_FILES ${PROJECT_SOURCE_DIR}/ui/*.ui)

qt5_wrap_ui(obsplay_UI ${obsplay_UI_FILES})

add_library(obsplay MODULE ${SRC_FILES} ${obsplay_UI})

if (UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    list(APPEND ADDITIONAL_LIBS ${X11_LIBRARIES})
endif()

target_link_libraries(obsplay ${frontend-tools_PLATFORM_LIBS} libobs obs-frontend-api Qt5::Widgets ${ADDITIONAL_LIBS})
set_target_properties(obsplay PROPERTIES FOLDER "plugins/obsplay")
install_obs_plugin_with_data(obsplay data)
