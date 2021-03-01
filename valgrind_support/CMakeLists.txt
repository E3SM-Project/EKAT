
add_executable(serial_hw serial_hw.cxx)
add_executable(mpi_hw mpi_hw.cxx)

add_custom_target(serial_sup ALL
  COMMAND valgrind --gen-suppressions=all ./serial_hw 2>&1 | ${CMAKE_CURRENT_SOURCE_DIR}/gen_sup.sh > ${CMAKE_BINARY_DIR}/serial.supp
  DEPENDS serial_hw
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )

add_custom_target(mpi_sup ALL
  COMMAND mpiexec -np 2 valgrind --gen-suppressions=all ./mpi_hw 2>&1 | ${CMAKE_CURRENT_SOURCE_DIR}/gen_sup.sh > ${CMAKE_BINARY_DIR}/mpi.supp
  DEPENDS mpi_hw
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
