# WinSock2_FOUND: if found
# WinSock2::ws2_32: imported module

include(FindPackageHandleStandardArgs)

find_library(WinSock2_LIBRARY NAMES ws2_32)
find_path(WinSock2_INCLUDE_DIR winsock2.h)

find_package_handle_standard_args(WinSock2
	FOUND_VAR WinSock2_FOUND
	REQUIRED_VARS WinSock2_LIBRARY WinSock2_INCLUDE_DIR
)

if (WinSock2_FOUND)
	add_library(WinSock2::ws2_32 UNKNOWN IMPORTED)
	set_target_properties(WinSock2::ws2_32 PROPERTIES
		IMPORTED_LOCATION ${WinSock2_LIBRARY}
		INTERFACE_INCLUDE_DIRECTORIES ${WinSock2_INCLUDE_DIR}
	)
endif()