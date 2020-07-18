if(WIN32)
  ocv_update(OPENCV_SETUPVARS_INSTALL_PATH ".")
  ocv_update(OPENCV_SCRIPT_EXTENSION ".cmd")
  ocv_update(OPENCV_SETUPVARS_TEMPLATE "setup_vars_win32.cmd.in")
else()
  ocv_update(OPENCV_SETUPVARS_INSTALL_PATH "bin")
  ocv_update(OPENCV_SCRIPT_EXTENSION ".sh")
  if(APPLE)
    ocv_update(OPENCV_SETUPVARS_TEMPLATE "setup_vars_macosx.sh.in")
  else()
    ocv_update(OPENCV_SETUPVARS_TEMPLATE "setup_vars_linux.sh.in")
  endif()
endif()

if(INSTALL_TO_MANGLED_PATHS)
  ocv_update(OPENCV_SETUPVARS_FILENAME "setup_vars_opencv-${OPENCV_VERSION}${OPENCV_SCRIPT_EXTENSION}")
else()
  ocv_update(OPENCV_SETUPVARS_FILENAME setup_vars_opencv4${OPENCV_SCRIPT_EXTENSION})
endif()

##### build directory
if(WIN32)
  set(__build_type "${CMAKE_BUILD_TYPE}")
  if(NOT __build_type)
    set(__build_type "Release")  # default
  endif()
  file(RELATIVE_PATH OPENCV_LIB_RUNTIME_DIR_RELATIVE_CMAKECONFIG "${OpenCV_BINARY_DIR}/" "${EXECUTABLE_OUTPUT_PATH}/${__build_type}/")
else()
  file(RELATIVE_PATH OPENCV_LIB_RUNTIME_DIR_RELATIVE_CMAKECONFIG "${OpenCV_BINARY_DIR}/" "${LIBRARY_OUTPUT_PATH}/")
endif()
set(OPENCV_PYTHON_DIR_RELATIVE_CMAKECONFIG "python_loader")  # https://github.com/opencv/opencv/pull/12977
configure_file("${OpenCV_SOURCE_DIR}/cmake/templates/${OPENCV_SETUPVARS_TEMPLATE}" "${CMAKE_BINARY_DIR}/tmp/setup_vars${OPENCV_SCRIPT_EXTENSION}" @ONLY)
file(COPY "${CMAKE_BINARY_DIR}/tmp/setup_vars${OPENCV_SCRIPT_EXTENSION}" DESTINATION "${CMAKE_BINARY_DIR}"
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

##### install directory
if(WIN32)
  file(RELATIVE_PATH OPENCV_LIB_RUNTIME_DIR_RELATIVE_CMAKECONFIG
      "${CMAKE_INSTALL_PREFIX}/${OPENCV_SETUPVARS_INSTALL_PATH}/" "${CMAKE_INSTALL_PREFIX}/${OPENCV_BIN_INSTALL_PATH}/")
else()
  file(RELATIVE_PATH OPENCV_LIB_RUNTIME_DIR_RELATIVE_CMAKECONFIG
      "${CMAKE_INSTALL_PREFIX}/${OPENCV_SETUPVARS_INSTALL_PATH}/" "${CMAKE_INSTALL_PREFIX}/${OPENCV_LIB_INSTALL_PATH}/")
endif()
file(RELATIVE_PATH OPENCV_PYTHON_DIR_RELATIVE_CMAKECONFIG
    "${CMAKE_INSTALL_PREFIX}/${OPENCV_SETUPVARS_INSTALL_PATH}/" "${CMAKE_INSTALL_PREFIX}/")
if(DEFINED OPENCV_PYTHON_INSTALL_PATH)
  set(__python_path "${OPENCV_PYTHON_INSTALL_PATH}")
elseif(DEFINED OPENCV_PYTHON_INSTALL_PATH_SETUPVARS)
  set(__python_path "${OPENCV_PYTHON_INSTALL_PATH_SETUPVARS}")
endif()
if(DEFINED __python_path)
  if(IS_ABSOLUTE "${__python_path}")
    set(OPENCV_PYTHON_DIR_RELATIVE_CMAKECONFIG "${__python_path}")
    message(WARNING "CONFIGURATION IS NOT SUPPORTED: validate setupvars script in install directory")
  else()
    ocv_path_join(OPENCV_PYTHON_DIR_RELATIVE_CMAKECONFIG "${OPENCV_PYTHON_DIR_RELATIVE_CMAKECONFIG}" "${__python_path}")
  endif()
else()
  if(DEFINED OPENCV_PYTHON3_INSTALL_PATH)
    ocv_path_join(OPENCV_PYTHON_DIR_RELATIVE_CMAKECONFIG "${OPENCV_PYTHON_DIR_RELATIVE_CMAKECONFIG}" "${OPENCV_PYTHON3_INSTALL_PATH}")
  else()
    set(OPENCV_PYTHON_DIR_RELATIVE_CMAKECONFIG "python_loader_is_not_installed")
  endif()
endif()
configure_file("${OpenCV_SOURCE_DIR}/cmake/templates/${OPENCV_SETUPVARS_TEMPLATE}" "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/install/${OPENCV_SETUPVARS_FILENAME}" @ONLY)
install(FILES "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/install/${OPENCV_SETUPVARS_FILENAME}"
    DESTINATION "${OPENCV_SETUPVARS_INSTALL_PATH}"
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    COMPONENT scripts)
