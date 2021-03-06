Linux
-----

Download and compilation
~~~~~~~~~~~~~~~~~~~~~~~~

If you are using a Debian-based system, install the (required) libraries first:

.. sourcecode::
    .

    apt-get install git git-core cmake g++ gfortran freeglut3-dev libsuitesparse-dev libglew1.5-dev libxerces-c-dev xsdcxx libmatio-dev

.. latexcode::
    .

    apt-get install cmake g++ gfortran freeglut3-dev libsuitesparse-dev libglew1.5-dev 

If you want to use fast saving / loading of Hermes entities, install

  - BSON
  
    - Clone the BSON Mongo driver git repository from git@github.com:l-korous/mongo-c-driver.git (if you don't know how, here is a tip: `Getting a Git Repository <http://git-scm.com/book/en/Git-Basics-Getting-a-Git-Repository>`_)
    - Compile and install using 'make install'

For thread caching memory allocator from Google, see
    
  - TCMalloc
    
      - Get TCMalloc from the SVN repository at http://code.google.com/p/gperftools/source/checkout
      - Make & install
  
To obtain the source code, clone the Git repository from Github::
  
    git clone git://github.com/hpfem/hermes.git

These two repositories are synchronized. For more advanced users we recommend to 
create a free account at `Github <http://github.com>`_ (if you do not have one yet),
fork the `Hermes repository <http://github.com/hpfem/hermes>`_, and then clone your 
Github copy of Hermes to your local computer. This will establish links between
your local copy and the master repository, and you'll become part of the Hermes 
network at Github.

Once you have a local copy of the Hermes repository on your computer, change dir 
to hermes/. There you will find a CMakeLists.txt file that contains the lines::

    # OpenMP
    # "-1" stands for using as many threads as is the number of available cores.
    # Please be aware that the variable OMP_NUM_THREADS, that is often used for this purpose, is ignored.
    set(NUM_THREADS -1)
    
    # HermesCommon
      set(HERMES_COMMON_DEBUG     YES)
		  set(HERMES_COMMON_RELEASE   YES)
      ...
      
    # Hermes2D:
    set(WITH_H2D                        YES)
      set(H2D_DEBUG               YES)
		  set(H2D_RELEASE             YES)
		  # Optional parts of the library.
		  set(H2D_WITH_GLUT 					YES)
      ...
      
    set(WITH_SUPERLU            NO)
    ...


Create a file called "CMake.vars" where you set all 
these variables according to your needs. Examples of CMake.vars files can
be found in the CMakeVars folder.
After that, type::

    cmake .
    make

If you have more than one CPU, you can use "make -jN" where N is
the number of CPUs of your computer.

Debugging with Eclipse
~~~~~~~~~~~~~~~~~~~~~~

To use eclipse as debugger, in the root folder of the project::

    mkdir eclipse_build
    cd eclipse_build
    cmake -G"Eclipse CDT4 - Unix Makefiles" -D CMAKE_BUILD_TYPE=Debug ../

In Eclipse:

    - Import project using Menu File->Import
    - Select General->Existing projects into workspace:
    - Browse where your build tree is and select the root build tree directory. 
    - Keep "Copy projects into workspace" unchecked.


Install Hermes
~~~~~~~~~~~~~~

::

    cmake -DCMAKE_INSTALL_PREFIX=~/usr .
    make
    make install
