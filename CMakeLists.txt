cmake_minimum_required(VERSION 3.24)
project(Coinflict LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the CMake prefix path to locate dotenv.cmake
set(CMAKE_PREFIX_PATH "/usr/local/lib/cmake")
find_package(laserpants_dotenv REQUIRED)

# Find other packages
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
pkg_search_module(POPPLER REQUIRED poppler-cpp)

# Paths (updated to external folder)
set(EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(IMGUI_DIR ${EXTERNAL_DIR}/imgui)
set(IMPLOT_DIR ${EXTERNAL_DIR}/implot)
set(IMGUI_FILE_DIALOG_DIR ${EXTERNAL_DIR}/ImGuiFileDialog)
set(IMGUI_DATE_PICKER_DIR ${EXTERNAL_DIR}/ImGuiDatePicker)
# Sources
set(SOURCES
    ../app/main.cpp
    ../app/lib/db/db.cpp
    ../app/lib/parser/parser.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_FILE_DIALOG_DIR}/ImGuiFileDialog.cpp
    ${IMPLOT_DIR}/implot.cpp
    ${IMPLOT_DIR}/implot_items.cpp
    ${IMGUI_DATE_PICKER_DIR}/ImGuiDatePicker.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${IMPLOT_DIR}
    ${IMGUI_FILE_DIALOG_DIR}
    ${IMGUI_DATE_PICKER_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ${POPPLER_INCLUDE_DIRS}
    ${DOTENV_CPP_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${GLFW_LIBRARIES}
    ${POPPLER_LIBRARIES}
    GL
    laserpants::dotenv
    pthread
    pq
)
