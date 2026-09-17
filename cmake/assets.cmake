set(ASSETS_IN_DIR "${CMAKE_SOURCE_DIR}/assets")
set(ASSETS_OUT_DIR "${CMAKE_BINARY_DIR}/assets")

add_custom_target(copy_assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${ASSETS_IN_DIR} ${ASSETS_OUT_DIR}
)
