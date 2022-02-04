if (NOT egvs_SUPP_FILES_DIR)
  set (egvs_SUPP_FILES_DIR ${CMAKE_BINARY_DIR})
endif()

add_executable(serial_hw serial_hw.cxx)

message ("serial supp: ${egvs_SUPP_FILES_DIR}/serial.supp")
add_custom_target(serial_sup ALL
  COMMAND valgrind --gen-suppressions=all ./serial_hw 2>&1 | ${CMAKE_CURRENT_SOURCE_DIR}/gen_sup.sh > ${egvs_SUPP_FILES_DIR}/serial.supp
  DEPENDS serial_hw
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )

if (egvs_MPIEXEC)
  add_executable(mpi_hw mpi_hw.cxx)
  add_custom_target(mpi_sup ALL
    COMMAND ${egvs_MPIEXEC} -n 2 valgrind --gen-suppressions=all ./mpi_hw 2>&1 | ${CMAKE_CURRENT_SOURCE_DIR}/gen_sup.sh > ${egvs_SUPP_FILES_DIR}/mpi.supp
    DEPENDS mpi_hw
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()
