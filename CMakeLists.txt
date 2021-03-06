project(hermes)
  cmake_minimum_required(VERSION 2.8)

  #---------------------------------------------------------------------------------------------
  # Main configuration options.
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  ### BEGIN DEFAULT CONFIGURATION ##############################################################

  # The values below are sane defaults to produce a build on all platforms,
  # if you want to override them, set them in CMake.vars

  # Global
  # Generate static libs (instead of dynamic)
  set(HERMES_STATIC_LIBS NO)
  
  # For Win64
  if(${CMAKE_CL_64})
    set(WIN64 YES)
  else(${CMAKE_CL_64})
    set(WIN64 NO)
  endif(${CMAKE_CL_64})
    
  # HermesCommon
  # These are ignored for MSVC (where debug / release are handled differently)
  set(HERMES_COMMON_DEBUG     YES)
  set(HERMES_COMMON_RELEASE   YES)

  # Hermes2D:
  set(WITH_H2D                  YES)
    # Release and debug versions.
    # These are ignored for MSVC (where debug / release are handled differently)
    set(H2D_DEBUG               YES)
    set(H2D_RELEASE             YES)
	
    # Optional parts of the library.
    set(H2D_WITH_GLUT           YES)
    set(H2D_WITH_TEST_EXAMPLES  YES)
	
    # Advanced settings.
    # Number of solution / filter components.
    # Minimum is 1, maximum 999.
    set(H2D_MAX_COMPONENTS 10)
    # Number of quadratures, recommended is 4.
    # Minimum is 2, maximum 999.
    set(H2D_MAX_QUADRATURES 4)
    #  This number has to be smaller than 21.
    set(H2D_MAX_HANGING_NODES_LEVEL 15)
    #  Use second derivatives (that are turned off by default).
    set(H2D_USE_SECOND_DERIVATIVES NO)
  
  # TC_MALLOC
  set(WITH_TC_MALLOC NO)

  # BSON 
  set(WITH_BSON NO)

  # MATIO
  set(WITH_MATIO NO)
  set(MATIO_WITH_HDF5 NO)
  
  # BFD
  set(WITH_BFD NO)

  #### Solvers ###
  # UMFPACK is chosen by default.
  set(WITH_UMFPACK YES)
  
  # PARALUTION
  set(WITH_PARALUTION NO)
  
  # MUMPS
  # Sequential version of MUMPS may be enabled by setting YES on the following line
  # and pointing to the package installation directory with MUMPS_ROOT. To enable the
  # parallel version, setting WITH_MPI to YES in addition to that should suffice. If link
  # problems arise, it may be that the automatically found SCALAPACK libraries (reported
  # during the CMake configuration phase) do not match those used when compiling MUMPS.
  # In this case, set SCALAPACK_LIBRARIES manually in CMake.vars and reconfigure Hermes.
  set(WITH_MUMPS              NO)
  # set(MUMPS_ROOT            root/dir/of/mumps/)
  # set(SCALAPACK_LIBRARIES   -lscalapack -lblacs)

  # Trilinos
  # Enable support for Trilinos solvers.
  set(WITH_TRILINOS           NO)
    
  ### Mesh format ###
  set(WITH_EXODUSII           NO)
  set(WITH_HDF5               NO)

  ### Others ###
  # Parallel execution.
    # (tells the linker to use parallel versions of the selected solvers, if available):
    #
    # Enable OpenMP for shared memory processing.
    #
    set(WITH_OPENMP             YES)

    # If MPI is enabled, the MPI library installed on the system should be found by
    # CMake automatically. If the found library doesn't match the one used to compile the
    # particular MPI-dependent package, the other two options should be used to specify it.
    #
    set(WITH_MPI                NO)
    # set(MPI_LIBRARIES         -lmpi)
    # set(MPI_INCLUDE_PATH      /usr/include/openmpi

    # PJLIB - memory pool implementation
    set(WITH_PJLIB               NO)
      
    # Optional Windows stacktrace
    set(WITH_WINDOWS_STACKWALKER NO)
    
  ################################## CURRENTLY NOT SUPPORTED ###################################
  #  \/   \/   \/   \/   \/   \/   \/   \/   \/   \/   \/   \/   \/   \/   \/   \/   \/   \/
  
  # SuperLU
    # First option enables sequential SuperLU, which should reside in SUPERLU_ROOT/lib
    # and SUPERLU_ROOT/include. If SUPERLU_ROOT is not provided, the library will be searched
    # for in system directories. If SUPERLU_MT is set to YES, the multithreaded version of
    # SuperLU will be used. In this case, there should be either superlu_mt_PTHREAD.a and/or
    # superlu_mt_OPENMP.a libraries present in the search path (the former will be chosen if
    # WITH_OPENMP is set to NO, the latter otherwise). There should also be a subdirectory
    # SLU_MT in the include directory since SuperLU_MT uses different versions of the same-named
    # header files as the serial library.
    set(WITH_SUPERLU            NO)
    set(SUPERLU_ROOT            /opt/SuperLU_4.0/)
    set(SUPERLU_MT              NO)

  # PETSc
    # If PETSc is enabled, PETSC_ROOT and PETSC_ARCH must be set as well;
    # if PETSc is disabled, PETSC_ROOT and PETSC_ARCH will be ignored.
    set(WITH_PETSC              NO)
    set(PETSC_ROOT              /opt/petsc/petsc-3.1-p7)
    set(PETSC_ARCH              petsc-arch-without-scalar-type)

  #  /\   /\   /\   /\   /\   /\   /\   /\   /\   /\   /\   /\   /\   /\   /\   /\   /\   /\
  ################################## CURRENTLY NOT SUPPORTED ###################################
  
  ### END DEFAULT CONFIGURATION ################################################################
  ##############################################################################################
  ##############################################################################################
  ### INTERNALS FOLLOW - DO NOT CHANGE #########################################################
    
  # Allow to override the default values in CMake.vars:
	include(CMake.vars OPTIONAL)
    include(../CMakeHermes.cmake OPTIONAL)
    
  set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
  if(WIN64)
    set(HERMES_COMMON_LIB  hermes_common_64)
  else()
    set(HERMES_COMMON_LIB  hermes_common)
  endif()
  
  if(${H2D_MAX_HANGING_NODES_LEVEL} MATCHES "[1-20]")
  else(${H2D_MAX_HANGING_NODES_LEVEL} MATCHES "[1-20]")
	message(FATAL_ERROR "H2D_MAX_HANGING_NODES_LEVEL has to be a number smaller than 21.")
  endif(${H2D_MAX_HANGING_NODES_LEVEL} MATCHES "[1-20]")
  
  if(${H2D_MAX_COMPONENTS} MATCHES "[1-999]")
  else(${H2D_MAX_COMPONENTS} MATCHES "[1-999]")
	message(FATAL_ERROR "H2D_MAX_COMPONENTS has to be a number between 1 and 999.")
  endif(${H2D_MAX_COMPONENTS} MATCHES "[1-999]")
  
  if(${H2D_MAX_QUADRATURES} MATCHES "[1-999]")
  else(${H2D_MAX_QUADRATURES} MATCHES "[1-999]")
	message(FATAL_ERROR "H2D_MAX_QUADRATURES has to be a number between 1 and 999.")
  endif(${H2D_MAX_QUADRATURES} MATCHES "[1-999]")
  
  #ADDED BY COLMAN
  IF (APPLE)
    IF (NOT CMAKE_SYSTEM_PROCESSOR MATCHES powerpc)
      IF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        ## It's Mac OS X.

        #This is causing problems when building the debug version.
        #SET (CMAKE_BUILD_TYPE "Release" CACHE STRING "Release is default." FORCE)

        #SET(CMAKE_OSX_ARCHITECTURES "i386" CACHE STRING
        #  "compiling for OSX only in i386 format. Library with the
        #   necessary symbol '_glutSetOption' was only available
        #   in /usr/lib/libglut.dylib, which happened to be a pure
        #   i386 library. Maybe fixed later." FORCE)
        
		IF (CMAKE_VERBOSE_MAKEFILE)
          MESSAGE(STATUS)
          MESSAGE(STATUS "*******************************************************************************")
          MESSAGE(STATUS "*        SUMMARY OF USED VARIABLES -> Details in AllVariables.txt             *")
          MESSAGE(STATUS "*******************************************************************************")
          MESSAGE(STATUS "COMPILING FOR OSX, platforms: ${CMAKE_OSX_ARCHITECTURES}")
          MESSAGE(STATUS "CMAKE_ROOT ........................... = ${CMAKE_ROOT}")
          MESSAGE(STATUS "CMAKE_HOME_DIRECTORY ................. = ${CMAKE_HOME_DIRECTORY}")
          MESSAGE(STATUS "CMAKE_VERSION ........................ = ${CMAKE_VERSION}")
          MESSAGE(STATUS "CMAKE_GENERATOR ...................... = ${CMAKE_GENERATOR}")
          MESSAGE(STATUS "CMAKE_MODULE_PATH .................... = ${CMAKE_MODULE_PATH}")
          MESSAGE(STATUS "CMAKE_HOST_SYSTEM .................... = ${CMAKE_HOST_SYSTEM}")
          MESSAGE(STATUS "CMAKE_HOST_SYSTEM_VERSION ............ = ${CMAKE_HOST_SYSTEM_VERSION}")
          MESSAGE(STATUS "CMAKE_HOST_SYSTEM_NAME ............... = ${CMAKE_HOST_SYSTEM_NAME}")
          MESSAGE(STATUS "CMAKE_HOST_SYSTEM_PROCESSOR .......... = ${CMAKE_HOST_SYSTEM_PROCESSOR}")
          MESSAGE(STATUS "CMAKE_HOST_UNIX ...................... = ${CMAKE_HOST_UNIX}")
          MESSAGE(STATUS "CMAKE_BUILD_TYPE ..................... = ${CMAKE_BUILD_TYPE}")
          MESSAGE(STATUS "CMAKE_C_COMPILER ..................... = ${CMAKE_C_COMPILER}")
          MESSAGE(STATUS "CMAKE_CXX_COMPILER ................... = ${CMAKE_CXX_COMPILER}")
        ENDIF()
      ENDIF ()
    ENDIF (NOT CMAKE_SYSTEM_PROCESSOR MATCHES powerpc)
    # Use modified search procedures for some libraries on MacOS.
    include(${CMAKE_HOME_DIRECTORY}/OSX.cmake)
  ENDIF (APPLE)
  # end ADDED BY COLMAN

  # Determine the number of cores.
  if(WIN32)
    set(NUM_THREADS "$ENV{NUMBER_OF_PROCESSORS}")
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    execute_process(COMMAND sysctl -a COMMAND grep "machdep.cpu.thread_count" COMMAND grep -o "[0-9]" OUTPUT_VARIABLE NUM_THREADS)
  else()
    set(cpuinfo_file "/proc/cpuinfo")
    if(EXISTS "${cpuinfo_file}")
      file(STRINGS "${cpuinfo_file}" procs REGEX "^processor.: [0-9]+$")
      list(LENGTH procs NUM_THREADS)
    else()
      set(NUM_THREADS 1)
    endif()
  endif()
  
  if(NOT(WITH_OPENMP))
    set(NUM_THREADS 1)
  endif(NOT(WITH_OPENMP))
  
  MESSAGE(STATUS "Hermes will by default be using ${NUM_THREADS} threads for calculation.")

  macro (DETERMINE_HERMES_COMMON_RELEASE_DEBUG HXD)
    if (WITH_${HXD})
      # In MSVC, Release/Debug is distinguished in a special way
      # (see hermes_common/cmake/BuildAndInstallScripts.cmake)
      if(NOT MSVC)
        if(${HXD}_DEBUG)
          set(HERMES_COMMON_DEBUG YES)
          set(HERMES_COMMON_LIB_DEBUG  ${HERMES_COMMON_LIB}-debug)
        endif(${HXD}_DEBUG)

        if(${HXD}_RELEASE)
          set(HERMES_COMMON_RELEASE YES)
          set(HERMES_COMMON_LIB_RELEASE ${HERMES_COMMON_LIB})
        endif(${HXD}_RELEASE)
      endif(NOT MSVC)
    endif (WITH_${HXD})
  endmacro(DETERMINE_HERMES_COMMON_RELEASE_DEBUG)

  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Disable all warnings and turn on only important ones:
    IF(DEFINED OLD_GCC)
      set(CMAKE_CXX_FLAGS "-w ${CMAKE_CXX_FLAGS} -fPIC")
    ELSE()
      set(CMAKE_CXX_FLAGS "-w ${CMAKE_CXX_FLAGS} -fPIC -std=c++11")
    ENDIF()
      set(CMAKE_CXX_FLAGS "-Wuninitialized -Wvla -Wsign-compare ${CMAKE_CXX_FLAGS}")

    set(HERMES_RELEASE_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG -O3")
    set(HERMES_DEBUG_FLAGS   "${CMAKE_CXX_FLAGS} -g -D_DEBUG")
    
    # look for libbfd
    find_library( BFD_LIBRARY  NAMES bfd libbfd bfd-2.23.2-system PATH /usr/lib /usr/lib64 )

    include(CheckCSourceCompiles)
    check_c_source_compiles(
    "#include <bfd.h>
    int main(void) {
    return 0;
    } " BFD_WORKS)

    if (BFD_LIBRARY AND BFD_WORKS AND WITH_BFD)
      set( HAVE_BFD YES )
    else()
      set(BFD_LIBRARY "")
    endif()
  endif()

  # This overrides CXX flags for MSVC
  if(MSVC)
    include(MSVC_FLAGS)
  endif(MSVC)

  if(H2D_WITH_TESTS OR HERMES_COMMON_WITH_TESTS)
    enable_testing()
    add_custom_target(test COMMAND ctest)
  endif(H2D_WITH_TESTS OR HERMES_COMMON_WITH_TESTS)

  #------------------------------------------------------------------------------
  # Find necessary packages according to the settings above.
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  if(WITH_TC_MALLOC)
    find_package(TCMALLOC REQUIRED)
    include_directories(${TCMALLOC_INCLUDE_DIR})
  endif(WITH_TC_MALLOC)
  
  if(WITH_PJLIB)
    find_package(PJLIB REQUIRED)
    include_directories(${PJLIB_INCLUDE_DIR})
  endif(WITH_PJLIB)
  
  IF(DEFINED BSON_LIBRARY)
    IF(DEFINED BSON_INCLUDE_DIR)
      include_directories(${BSON_INCLUDE_DIR})
    ELSE()
      MESSAGE(FATAL_ERROR When BSON_LIBRARY is defined, so must be BSON_INCLUDE_DIR.)
    ENDIF()
  ELSE()
    if(WITH_BSON)
      find_package(BSON REQUIRED)
      include_directories(${BSON_INCLUDE_DIR})
    endif(WITH_BSON)
  ENDIF()

  IF(DEFINED MATIO_LIBRARY)
    IF(DEFINED MATIO_INCLUDE_DIR)
      include_directories(${MATIO_INCLUDE_DIR})
    ELSE()
      MESSAGE(FATAL_ERROR When MATIO_LIBRARY is defined, so must be MATIO_INCLUDE_DIR.)
    ENDIF()
  ELSE()
    if(WITH_MATIO)
      find_package(MATIO REQUIRED)
      include_directories(${MATIO_INCLUDE_DIR})
    endif(WITH_MATIO)
  ENDIF()

  find_package(XSD REQUIRED)
  find_package(XERCES REQUIRED)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(WITH_STACKTRACE)
      find_package(EXECINFO)
    endif(WITH_STACKTRACE)
  endif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  
  # Solvers.
  if(WITH_UMFPACK)
    find_package(UMFPACK REQUIRED)
    include_directories(${UMFPACK_INCLUDE_DIRS})
  endif(WITH_UMFPACK)
  
  if(WITH_MATIO AND MATIO_WITH_HDF5)
    find_package(HDF5 REQUIRED)
    include_directories(${HDF5_INCLUDE_DIR})
  else()
    set(MATIO_WITH_HDF5 NO)
  endif()
  
  IF(DEFINED PARALUTION_LIBRARY)
    IF(DEFINED PARALUTION_INCLUDE_DIR)
      include_directories(${PARALUTION_INCLUDE_DIR})
    ELSE()
      MESSAGE(FATAL_ERROR When PARALUTION_LIBRARY is defined, so must be PARALUTION_INCLUDE_DIR.)
    ENDIF()
  ELSE()
    if(WITH_PARALUTION)
      find_package(PARALUTION REQUIRED)
      include_directories(${PARALUTION_INCLUDE_DIR})
    endif(WITH_PARALUTION)
  ENDIF()
  
  if(MSVC)
    if(WITH_PETSC OR WITH_TRILINOS OR WITH_SUPERLU)
        # The search for CLAPACK and associated libraries will be performed
        # every time the project is configured in MSVC (TODO: Find out if
        # CLAPACK_LIBRARY, BLAS_LIBRARY, F2C_LIBRARY and CLAPACK_INCLUDE_DIRS
        # could be cached to improve subsequent configuration performance).
        find_package(CLAPACK REQUIRED)
        include_directories(${CLAPACK_INCLUDE_DIRS})
    endif(WITH_PETSC OR WITH_TRILINOS OR WITH_SUPERLU)
    if(WITH_MUMPS)
      find_package(WINBLAS REQUIRED)
    endif(WITH_MUMPS)
  else(MSVC)
    # All third party solvers require BLAS and LAPACK (both found in the package LAPACK).
    # FindLAPACK is Fortran-only so Fortran must be enabled as well.
    if(WITH_UMFPACK OR WITH_PETSC OR WITH_MUMPS OR WITH_TRILINOS OR WITH_SUPERLU)
      if (NOT LAPACK_FOUND)
          enable_language(Fortran)
          find_package(LAPACK REQUIRED)
              
          # If no error occured, LAPACK library has been found. Save the path to
          # it to cache, so that it will not be searched for during next 'cmake .'
          set(LAPACK_LIBRARIES  ${LAPACK_LIBRARIES}
                  CACHE STRING  "Path to LAPACK/BLAS libraries.")
          set(LAPACK_FOUND      YES
                  CACHE STRING  "Have LAPACK/BLAS libraries been found?")
      endif (NOT LAPACK_FOUND)
      
      add_definitions(-DWITH_BLAS)  
    endif(WITH_UMFPACK OR WITH_PETSC OR WITH_MUMPS OR WITH_TRILINOS OR WITH_SUPERLU)
  endif(MSVC)
  
  if(WITH_PETSC)
    find_package(PETSC REQUIRED)
    # For some header files, PETSc uses the same name for both real and complex
    # versions (they only reside in different directories), so we can't globally
    # set the architecture-specific include directory here, but rather on a
    # per-target basis.
    include_directories(${PETSC_INCLUDE_DIRS})
    include_directories(${PETSC_CPLX_INCLUDE_DIRS})
    set(PETSC_LIBRARIES ${PETSC_CPLX_LIBRARIES})
  endif(WITH_PETSC)

  if(WITH_MUMPS)
    find_package(MUMPS REQUIRED)
    if(WITH_MPI)
      find_package(SCALAPACK REQUIRED) # parallel version of MUMPS needs ScaLapack+BLACS
    endif(WITH_MPI)
    include_directories(${MUMPS_INCLUDE_DIR})
    set(MUMPS_LIBRARIES ${MUMPS_CPLX_LIBRARIES})
    LIST(APPEND MUMPS_LIBRARIES ${MUMPS_REAL_LIBRARIES})
  endif(WITH_MUMPS)

  if(WITH_TRILINOS)
    find_package(TRILINOS REQUIRED)
    include_directories(${TRILINOS_INCLUDE_DIR})
  endif(WITH_TRILINOS)
  
  if(WITH_SUPERLU)
    find_package(SUPERLU REQUIRED)
    include_directories(${SUPERLU_INCLUDE_DIR})
    if(SUPERLU_MT)
      add_definitions(-DSLU_MT)
    endif(SUPERLU_MT)
  endif(WITH_SUPERLU)

  # SMP.
  if(WITH_OPENMP)
    find_package(OpenMP REQUIRED)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  endif(WITH_OPENMP)

  if(WITH_EXODUSII)
    find_package(EXODUSII REQUIRED)
    include_directories(${EXODUSII_INCLUDE_DIR})
  endif(WITH_EXODUSII)

  # If using any package that requires MPI (e.g. parallel versions of MUMPS, PETSC).
  if(WITH_MPI)
    if(NOT MPI_LIBRARIES OR NOT MPI_INCLUDE_PATH) # If MPI was not defined by the user
      find_package(MPI REQUIRED)
    else(NOT MPI_LIBRARIES OR NOT MPI_INCLUDE_PATH)
      if(NOT EXISTS MPI_INCLUDE_PATH)
        message(FATAL_ERROR "MPI include directory ${MPI_INCLUDE_PATH} not found")
      endif(NOT EXISTS MPI_INCLUDE_PATH)
    endif(NOT MPI_LIBRARIES OR NOT MPI_INCLUDE_PATH)
    include_directories(${MPI_INCLUDE_PATH})
  endif(WITH_MPI)
  
  IF(WITH_WINDOWS_STACKWALKER)
    IF(MSVC)
      find_package(STACK_WALKER REQUIRED)
      include_directories(${STACK_WALKER_INCLUDE_DIR})
    ENDIF(MSVC)
  ENDIF(WITH_WINDOWS_STACKWALKER)

  # Match debug or release version of all other libraries.
  DETERMINE_HERMES_COMMON_RELEASE_DEBUG(H2D)
  
  if(NOT H2D_WITH_GLUT)
    set(NOGLUT  YES)
  endif(NOT H2D_WITH_GLUT)
  
  if(HAVE_BFD)
    find_package(LIBIBERTY REQUIRED)
    include_directories(${LIBIBERTY_INCLUDE_DIR})
  endif()

  # Main CMakeLists.txt for each library are in the following directories.
  add_subdirectory(hermes_common)
    
  if(WITH_H2D)
    add_subdirectory(hermes2d)
  endif(WITH_H2D)

  #-------
  # Report.
  #~~~~~~~~
  message("\nHermes Configuration results")
  message("---------------------")
  message("Build static libs: ${HERMES_STATIC_LIBS}")
  if(WIN32)
    message("Build for Win64: ${WIN64}")
  endif(WIN32)
  
  message("\n-------Features-------")
  message("Build with OpenMP: ${WITH_OPENMP}")
  message("Build with TCMalloc: ${WITH_TC_MALLOC}")
  message("Build with BSON: ${WITH_BSON}")
  message("Build with MATIO: ${WITH_MATIO}")
  if(${WITH_MATIO})
    message(" MATIO with HDF5: ${MATIO_WITH_HDF5}")
  endif()
  if(${WITH_MPI})
    message("Build with MPI: ${WITH_MPI}")
  endif()
  message("Build with EXODUSII: ${WITH_EXODUSII}")
  
  message("\n-------Solvers-------")
  message("Build with UMFPACK: ${WITH_UMFPACK}")
  message("Build with PARALUTION: ${WITH_PARALUTION}")
  message("Build with PETSC: ${WITH_PETSC}")
  message("Build with MUMPS: ${WITH_MUMPS}")
  message("Build with SUPERLU${MT}: ${WITH_SUPERLU}")
  message("Build with TRILINOS: ${WITH_TRILINOS}")
  
  message("\n-----Hermes common-----")
  message(" Debug version: ${HERMES_COMMON_DEBUG}")
  message(" Release version: ${HERMES_COMMON_RELEASE}")

  message("\n-----Hermes2D-----")
  message(" Build Hermes2D: ${WITH_H2D}")
  if(WITH_H2D)
    message(" Debug version: ${H2D_DEBUG}")
    message(" Release version: ${H2D_RELEASE}")
    message(" Test examples: ${H2D_WITH_TEST_EXAMPLES}")
    message(" Hermes2D with OpenGL: ${H2D_WITH_GLUT}")
  endif(WITH_H2D)
  message("---------------------")
  message("\n")
