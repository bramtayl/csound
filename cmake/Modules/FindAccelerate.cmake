# Accelerate_FOUND: if found
# Accelerate::accelerate: imported module
include(FindPackageHandleStandardArgs)

find_library(Accelerate_LIBRARY Accelerate)
find_path(Accelerate_INCLUDE_DIR "Accelerate/Accelerate.h")

find_package_handle_standard_args(Accelerate
	FOUND_VAR Accelerate_FOUND
	REQUIRED_VARS Accelerate_LIBRARY Accelerate_INCLUDE_DIR
)

if(Accelerate_FOUND AND NOT TARGET Accelerate::accelerate)
	add_library(Accelerate::accelerate UNKNOWN IMPORTED)
	set_target_properties(Accelerate::accelerate PROPERTIES
		IMPORTED_LOCATION ${Accelerate_LIBRARY}
		INTERFACE_INCLUDE_DIRECTORIES ${Accelerate_INCLUDE_DIR}
	)
endif()

mark_as_advanced(Accelerate_LIBRARY Accelerate_INCLUDE_DIR)
