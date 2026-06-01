idf_component_register(SRC_DIRS "src/original" "src"
                       INCLUDE_DIRS "src" "src/original"
                       REQUIRES usb esp_ringbuf)

include(package_manager)
cu_pkg_define_version(${CMAKE_CURRENT_LIST_DIR})
