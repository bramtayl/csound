# WinMM_FOUND: if found
# WinMM::winmm: imported module

include(FindPackageHandleStandardArgs)

find_library(WinMM_LIBRARY NAMES winmm)
find_path(WinMM_INCLUDE_DIR mmeapi.h)

find_package_handle_standard_args(WinMM
	FOUND_VAR WinMM_FOUND
	REQUIRED_VARS WinMM_LIBRARY WinMM_INCLUDE_DIR
)

if (WinMM_FOUND)
	add_library(WinMM::winmm UNKNOWN IMPORTED)
	set_target_properties(WinMM::winmm PROPERTIES
		IMPORTED_LOCATION ${WinMM_LIBRARY}
		INTERFACE_INCLUDE_DIRECTORIES ${WinMM_INCLUDE_DIR}
	)
endif()