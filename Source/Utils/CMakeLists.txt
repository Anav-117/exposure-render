PROJECT(MeshCreation)

IF(NOT VTK_BINARY_DIR)
FIND_PACKAGE(VTK)
IF(NOT VTK_DIR)
  MESSAGE(FATAL_ERROR "Please set VTK_DIR.")
ENDIF(NOT VTK_DIR)
INCLUDE(${VTK_USE_FILE})
ENDIF(NOT VTK_BINARY_DIR)


SET(MeshCreationSources 
	# Utils
	MeshCreation.cpp
)

add_executable(MeshCreation ${MeshCreationSources})

TARGET_LINK_LIBRARIES(MeshCreation
    ${VTK_LIBRARIES}
)

