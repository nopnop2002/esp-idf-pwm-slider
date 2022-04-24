set(COMPONENT_SRCS "main.c" "http_server.c")
set(COMPONENT_ADD_INCLUDEDIRS ".")

if(IDF_TARGET STREQUAL "esp32s2")
	message(FATAL_ERROR "This target does not support MCPWM")
endif()

if(IDF_TARGET STREQUAL "esp32c3")
	message(FATAL_ERROR "This target does not support MCPWM")
endif()

register_component()
