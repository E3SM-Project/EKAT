
add_executable(gen_sup_serial serial.cxx)
add_executable(gen_sup_mpi_cxx mpi.cxx)
add_executable(gen_sup_mpi_f90 mpi.f90)

add_custom_target(serial_sup ALL
  COMMAND valgrind --gen-suppressions=all gen_sup_serial 2>&1 | ${CMAKE_CURRENT_SOURCE_DIR}/gen_sup.sh > ${CMAKE_BINARY_DIR}/serial.supp
  DEPENDS gen_sup_serial
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )

add_custom_target(mpi_sup ALL
  COMMAND mpiexec -np 2 valgrind --gen-suppressions=all $<TARGET_FILE:gen_sup_mpi_cxx> 2>&1 | ${CMAKE_CURRENT_SOURCE_DIR}/gen_sup.sh > ${CMAKE_BINARY_DIR}/mpi.supp
  COMMAND mpiexec -np 2 valgrind --gen-suppressions=all $<TARGET_FILE:gen_sup_mpi_f90> 2>&1 | ${CMAKE_CURRENT_SOURCE_DIR}/gen_sup.sh >> ${CMAKE_BINARY_DIR}/mpi.supp
  DEPENDS gen_sup_mpi_cxx gen_sup_mpi_f90
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
