Windows
----------

These installation instructions have been tested with Microsoft Visual Studio 2005, 2008, 2010, 2012, and with MinGW

- Probably with small modifications, they should also work for NMake
- If you would like to use NMake and you run in troubles, drop us a line to `Hermes2D mailing list <http://groups.google.com/group/hermes2d/>`_.
- On windows, a crucial file to edit is CMake.vars.example (see further).

Dependency check-list - overview
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  - You need to install dependent libraries into either common directory, or separate directories. This directory, further called 'dependencies' (stands for the particular directory in the case of particular dependency), has to have three subdirectories as follows. The choice of having a single common directory, or separate ones is up to you.

    - 'dependencies'\\include: Header files (\*.h) of dependency libraries.
    - 'dependencies'\\lib: Library files (\*.lib) of dependency libraries.   
    - 'dependencies'\\bin: Binary modules (\*.dll) of dependency libraries.
    - be sure to include a directory 'dependecies'\\bin into the 'PATH' environment variable (you need to include all of them if you chose to have separate ones for various dependencies).
  - For the 64-bit version, if you want to use it side-by-side to the 32-bit one, you can create a subdirectory 'x64' in the 'lib' folder where you will be putting the 64-bit dependency libraries.
  
Dependency check-list - 32-bit
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This list works for 32-bit version of Hermes. See the section for 64-bit version if that is the one you are interested in.
Please note that e.g. TCMalloc, BSON, UMFPACK are also 'optional', but to get the most performance out of Hermes, they are recommended.

  - CMAKE
  
    - Download CMAKE installer(http://www.cmake.org/files/v2.8/cmake-2.8.3-win32-x86.exe) and install it.

  - UMFPACK

    - MinGW used for compiling AMD and UMFPACK: `Download MinGW <http://sourceforge.net/projects/mingw/>`_.
    - after installing MinGW, add 'your-minGW-installation-directory'/bin to system PATH.
    - download latest `SuiteSparse_config <http://www.cise.ufl.edu/research/sparse/SuiteSparse_config/>`_, `AMD <http://www.cise.ufl.edu/research/sparse/amd/>`_, and `UMFPACK <http://www.cise.ufl.edu/research/sparse/umfpack/>`_ to  A SINGLE PARENT DIRECTORY (this requirement is one of UMFPACK).
    - Add the following lines at the end of file SuiteSparse_config\\SuiteSparse_config.mk:

      - CC = gcc
      - CXX = gcc
      - UMFPACK_CONFIG = -DNBLAS
      - RANLIB = echo
      - LIB = -lm
    
    - Open all files called 'Makefile' from all three directories and replace all ';' symbols in them with the Windows equivalent '&'
    - Copy SuiteSparse_config\\SuiteSparse_config.h to 'include' directory
    - Copy SuiteSparse_config\\libsuitesparseconfig.a to 'lib' directory and change its extension to Windows equivalent '.lib'.
    - Copy AMD\\Include\\amd.h, AMD\\Include\\amd_internal.h, and AMD\\Lib\\libamd.a to 'include', and 'lib' dependecy directories respectively. Change the libamd.a's extension to '.lib'
    - Copy UMFPACK\\Include\\* to 'include'
    - Copy UMFPACK\\Lib\\libumfpack.a to 'lib' directory and change its extension to Windows equivalent '.lib'.

  - XERCES

    - Download Xerces 3.1.1 source code from http://xerces.apache.org/xerces-c/download.cgi.
    - Build using your favorite compiler.
    - Copy all bin files to 'bin' dependencies directory
    - Copy all header files to 'include' dependencies directory
    - Copy the lib files to 'lib' dependencies directory

  - XSD
    - Download XSD library from http://www.codesynthesis.com/download/xsd/3.3/windows/i686/xsd-3.3.0-i686-windows.zip, instructions how to build the library are available at http://wiki.codesynthesis.com/Using_XSD_with_Microsoft_Visual_Studio.
    - Copy all bin files to 'bin' dependencies directory
    - Copy all header files to 'include' dependencies directory

  - OpenGL support (optional)

    - FREEGLUT 

      - Download freeglut 2.4.0 (http://freeglut.sourceforge.net/) and unpack it.
      - Open the your_freeglut_2.4.0_root\\freeglut.DSP file in Visual Studio and convert it to a newer format.
      - Compile Debug or Release version. Debug version is recommended in a case of debugging.
      - Copy 'freeglut.dll', 'freeglut.h', and 'freeglut.lib' to 'bin', 'include\\GL', and 'lib' dependency directories, respectively/.

    - GLEW

      - Download glew Win32 precompiled binaries ver.1.5.4 (http://glew.sourceforge.net/) and unpack it.
      - Copy 'my_glew_root\\bin\\glew32.dll', 'my_glew_root\\include\\GL\\\*.h', and 'my_glew_root\\lib\\glew32.lib' to 'bin', 'include\\GL', and 'lib' dependency directories respectively.
      
    - PTHREAD(2.9.1)

    - Download appropriate files (ftp://sourceware.org/pub/pthreads-win32/prebuilt-dll-2-9-1-release/).
    - Copy 'dll\\x86\\pthreadVCE2.dll', 'include\\\*.h' and 'lib\\x86\\pthreadVCE2.lib' to 'bin', 'include', and 'lib' dependecy directories respectively.
    
  

  - The rest is optional. If a directive WITH_BSON is *not* used, this step including all sub-steps can be skipped and you can proceed to `build Hermes <win.html#building-hermes>`_.
	
    - MATIO (1.5.2)
      
      - Download HDF5 <http://www.hdfgroup.org/ftp/HDF5/releases/hdf5-1.8.7/obtain5187.html>`_.
      - Install HDF5, note the path (you will need it for MATIO)
      - Download MATIO from <http://sourceforge.net/projects/matio/>`_.
      - Open the sln file in the folder visual_studio
      - Add to the Include Directories under the libmatio project settings the directory where you installed HDF5's headers
      - Add to the Libraries Directories under the libmatio project settings the directory where you installed HDF5's libs
      - Add to the linker linking to "libszip.lib"
      - (Fix MATIO error) Open the file zconf.h and on the line 287 change #if 1 to #if 0.
      - build, copy visual_studio/*.h and src/*.h to 'include' folder, visual_studio/Release/libmatio.lib to 'lib', visual_studio/Release/libmatio.dll to 'bin' folders.
      
    - BSON
    
      - Clone the BSON Mongo driver git repository from git@github.com:l-korous/mongo-c-driver.git (if you don't know how, here is a tip:`Getting a Git Repository <http://git-scm.com/book/en/Git-Basics-Getting-a-Git-Repository>`_)
      
      - Download SCONS build tool from http://sourceforge.net/projects/scons/files/latest/download?source=files
      - Install SCONS (you need to have PYTHON installed for that), run it (e.g. issuing C:\Python27\Scripts\scons.bat) in the BSON Mongo driver root directory
      
        - Use flags --m32 and --c99 ("C:\Python27\Scripts\scons.bat --c99 --m32")
        
      - Once compiled (should take seconds at most), copy src/bson.h to your 'include' dependency directory, bson.lib to 'lib', and bson.dll to 'bin' directories.

    
    - TCMalloc
    
      - Get TCMalloc from the SVN repository at http://code.google.com/p/gperftools/source/checkout
      - Open gperftools.sln in your Visual Studio, build the appropriate version (default works fine - just select Debug/Release)
      - Copy Win32\"Release/Debug"\libtcmalloc_minimal.dll to 'bin' dependency directory, Win32\"Release/Debug"\libtcmalloc_minimal.lib to 'lib' dependency directory
      - Copy the contents of src/google to 'include' dependency directory
    
    - ExodusII

      - Download sources of version 4.9.3 (http://sourceforge.net/projects/exodusii/) and unpack 'exodusii'
      - Add the following line to the file 'my_exodusii_root\\CMakeLists.txt' as:

        ::

            PROJECT(Exodusii)
            SET(NETCDF_INCLUDE_DIR "my_netcdf_root/libsrc4")    
            # add this line; 

        be sure to use a slash '/' instead of a backslash '\\'. 

      - Generate MSVC project files using CMAKE in command prompt as:

        ::

            cmake . -G "Visual Studio 9 2008"    # MSVC2008 user 
            cmake . -G "Visual Studio 10"        # MSVC2010 user 

        If you have Cygwin installed, make sure that you are using the windows version of cmake. 

      - Open a SLN file 'my_exodusii_root/ExodusII.sln' in MSVC08/10
      - Switch to 'Release' version
      - Build a project 'exoIIv2c': this will create a LIB file in 'my_exodusii_root\\cbind\\Release'
      - Copy 'exoIIv2c.lib' to 'lib' dependency directory structure
      - Copy 'my_exodusii_root\\cbind\\include\\exodusII.h and exodusII_ext.h' to 'include' dependency directory

    - CLAPACK

      - First, you need to install CLAPACK/CBLAS:
      - Download the file clapack-3.2.1-CMAKE.tgz from http://www.netlib.org/clapack/.
      - Use cmake to configure and build the debug version of clapack.
      - Copy '\\clapack-3.2.1-CMAKE\\BLAS\\SRC\\Debug\\blas.lib', '\\clapack-3.2.1-CMAKE\\F2CLIBS\\libf2c\\Debug\\libf2c.lib', and '\\clapack-3.2.1-CMAKE\\SRC\\Debug\\lapack.lib' to 'lib' dependency directory.
      - Copy the contains of '\\clapack-3.2.1-CMAKE\\INCLUDE\\' to 'include' dependency directory.

  
Dependency check-list - 64-bit
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  Only the most important dependencies are described here for the 64-bit version. For all others, all you must do is compile the 64-bit version, or get it in any other way and link it to Hermes instead of the 32-bit one.
  
  - CMAKE

    - Download CMAKE installer(http://www.cmake.org/files/v2.8/cmake-2.8.3-win32-x86.exe) and install it.

  - PTHREAD(2.9.1)

    - Download appropriate files (ftp://sourceware.org/pub/pthreads-win32/prebuilt-dll-2-9-1-release/).
    - Copy 'dll\\x64\\pthreadVCE2.dll', 'include\\\*.h' and 'lib\\x64\\pthreadVCE2.lib' to 'bin', 'include', and 'lib' dependecy directories respectively.

  - UMFPACK

    - MinGW used for compiling AMD and UMFPACK: `Download MinGW <http://sourceforge.net/projects/mingw/>`_.
    - Just use 64-bit MinGW and provide the flag "-m64", otherwise it is the same as in Win32 version.

  - XERCES

    - Download Xerces 3.1.1 source code from http://xerces.apache.org/xerces-c/download.cgi.
    - Build using your favorite compiler for 64-bit.
    - Copy all bin files to 'bin' dependencies directory
    - Copy all header files to 'include' dependencies directory
    - Copy the lib files to 'lib' dependencies directory
    
    
  - XSD
    - Download XSD library from http://www.codesynthesis.com/download/xsd/3.3/windows/i686/xsd-3.3.0-i686-windows.zip, instructions how to build the library are available at http://wiki.codesynthesis.com/Using_XSD_with_Microsoft_Visual_Studio.
    - Build the x64 version
    - Copy all bin files to 'bin' dependencies directory
    - Copy all header files to 'include' dependencies directory

  - OpenGL support (optional)

    - FREEGLUT 

      - Download freeglut 2.4.0 (http://freeglut.sourceforge.net/) and unpack it.
      - Open the your_freeglut_2.4.0_root\\freeglut.DSP file in Visual Studio and convert it to a newer format.
      - Compile Debug or Release version (x64 platform). Debug version is recommended in a case of debugging.
      - Copy 'freeglut.dll', 'freeglut.h', and 'freeglut.lib' to 'bin', 'include\\GL', and 'lib' dependency directories, respectively/.

    - GLEW

      - Download glew x64 precompiled binaries (http://glew.sourceforge.net/) and unpack it.
      - Copy 'my_glew_root\\bin\\glew32.dll', 'my_glew_root\\include\\GL\\\*.h', and 'my_glew_root\\lib\\glew32.lib' to 'bin', 'include\\GL', and 'lib' dependency directories respectively.
 	
  - The rest is optional. If a directive WITH_BSON is *not* used, this step including all sub-steps can be skipped and you can proceed to `build Hermes <win.html#building-hermes>`_.
  
    - MATIO (1.5.2)
      
      - Just follow the 32-bit version instructions and download HDF5 for x64, and also when building MATIO, build the x64 version.
      
    - TCMalloc
    
      - Get TCMalloc from the SVN repository at http://code.google.com/p/gperftools/source/checkout
      - Open gperftools.sln in your Visual Studio, build the appropriate version (default works fine - just select Debug/Release)
      - Copy x64\"Release/Debug"\libtcmalloc_minimal.dll to 'bin' dependency directory, x64\"Release/Debug"\libtcmalloc_minimal.lib to 'lib' dependency directory
      - Copy the contents of src/google to 'include' dependency directory
      
    - BSON
    
      - Clone the BSON Mongo driver git repository from git@github.com:l-korous/mongo-c-driver.git (if you don't know how, here is a tip:`Getting a Git Repository <http://git-scm.com/book/en/Git-Basics-Getting-a-Git-Repository>`_)
      - Download SCONS build tool from http://sourceforge.net/projects/scons/files/latest/download?source=files
      - Install SCONS (you need to have PYTHON installed for that), run it (e.g. issuing C:\Python27\Scripts\scons.bat) in the BSON Mongo driver root directory
      
        - Use the flag --c99 ("C:\Python27\Scripts\scons.bat --c99")
        
      - Once compiled (should take seconds at most), copy src/bson.h to your 'include' dependency directory, bson.lib to 'lib', and bson.dll to 'bin' directories.
    
Building Hermes
~~~~~~~~~~~~~~~

 In order to build the library and examples, you need to:

 - Prepare dependecy libraries, see 'Dependency Check-list'.
 - Copy a file 'CMake.vars.example' to 'CMake.vars'. The file contains settings for the project.
 - Modify the file 'CMake.vars'. For example, you 
   could set the first line as::

       set(DEP_ROOT "../dependencies")

 - In the root Hermes directory, to create project files by running CMAKE from a command prompt::

       cmake . -G "Visual Studio 8 2005"  # MSVC2005 user
       cmake . -G "Visual Studio 9 2008"  # MSVC2008 user
       cmake . -G "Visual Studio 10"      # MSVC2010 user
       cmake . -G "Visual Studio 11"      # MSVC2012 user
       cmake . -G "MinGW Makefiles"       # MinGW user

   If you have Cygwin installed, your might have an error "Coulld not create named generator Visual Studio 10". This is because your 
   cmake path is contaminated by Cygwin's cmake. Try to use absolute path for windows cmake.exe. 
   
 - Open the SLN file 'hermes.sln' and build Hermes.

Configuration options
~~~~~~~~~~~~~~~~~~~~~

 Hermes is configured through preprocessor directives. Directives are generated by CMAKE and your settings might be overriden by CMAKE. The directives are:

  - H2D_WITH_GLUT : If the line in your CMake.vars "set(H2D_WITH_GLUT NO)" is uncommented, it excludes GLUT-dependant parts. This replaces viewers with an empty implementation that does nothing if invoked. If used, the library 'freeglut.lib' does not need to be linked.
  
  - H2D_WITH_TEST_EXAMPLES : Produce project files for the test examples, which are a quick hands-on introduction to how Hermes works.

Using Hermes
~~~~~~~~~~~~
 
In order to use Hermes in your project, you need to do the following steps. Steps has 5, 6, and 7 to be repeated for every configuration, i.e., Debug, Release. Except the step 7b, this can be done easily by setting the drop-down Configuration to 'All configurations' in the Project Property dialog.

  - Prepare Hermes to be buildable by MSVC, see 'Building Hermes'.
  - Create your project in MSVC. Set the project to be an empty Win32 console project.
  - Add directories 'dependencies\\lib' to additional library directories (<right click on your project>\\Properties\\Configuration Properties\\Linker\\Additional Library Directories).
  - Add also the directory where you copied Hermes libraries to as an additional library directory. This would probably be the variable CMAKE_INSTALL_PREFIX in your CMake.vars file.
  - Add 'include "hermes2d.h"', make sure that your CMAKE_INSTALL_PREFIX is among Include Directories settings in your compiler.
  - Add the dependencies\\include directory (and possibly other directories where you copied dependency headers) using
  
    - Project -> Properties -> Configuration Properties -> VC++ Directories -> Include Directories

  - Deny (Ignore) warnings that are not indicating anything dangerous:

    - Ignore warnings about STL in DLL by denying a warning 4251 (<right click on your project>\\Properties\\Configuration Properties\\C/C++\\Advanced\\Disable Specific Warnings, enter 4251).
    - Ignore warnings about standard functions that are not safe (<right click on your project>\\Properties\\Configuration Properties\\C/C++\\Preprocessor\\Preprocessor Definitions, add _CRT_SECURE_NO_WARNINGS).
    - Also ignore any template instantiation warnings
  - Resolve unresolved linker error in Xerces
    - http://stackoverflow.com/questions/10506582/xerces-c-unresolved-linker-error