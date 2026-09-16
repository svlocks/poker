if(NOT DEFINED BUILD_DIR OR NOT DEFINED SOURCE_DIR)
  message(FATAL_ERROR "Pass BUILD_DIR and SOURCE_DIR")
endif()
set(install_root "${BUILD_DIR}/install-test")
file(REMOVE_RECURSE "${install_root}")
execute_process(COMMAND "${CMAKE_COMMAND}" --install "${BUILD_DIR}"
  --prefix "${install_root}/prefix" --config "${CONFIG}"
  COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${CMAKE_COMMAND}" -S "${SOURCE_DIR}/tests/consumer"
  -B "${install_root}/consumer" "-DCMAKE_PREFIX_PATH=${install_root}/prefix"
  -DCMAKE_BUILD_TYPE=Release COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${CMAKE_COMMAND}" --build "${install_root}/consumer"
  --config Release COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${install_root}/consumer"
  -C Release --output-on-failure --no-tests=error COMMAND_ERROR_IS_FATAL ANY)
