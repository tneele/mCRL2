.. _build-prerequisites:

Prerequisites
=============

.. include:: toggle-platform.inc

.. toctree::
   :hidden:

   boost

To build the source code, the first requirement is to have a compiler, and,
optionally, the necessary tools to build the documentation.
To build the documentation, the following is needed:

.. |python| replace:: *Python 2.5.1*
.. _python: http://www.python.org

.. |cmake| replace:: *CMake 2.8.6*
.. _cmake: http://www.cmake.org/cmake/resources/software.html

.. |cmakeosx| replace:: *CMake 2.8.12*
.. _cmakeosx: http://www.cmake.org/cmake/resources/software.html

.. |doxygen| replace:: *Doxygen 1.7.4*
.. _doxygen: http://www.doxygen.org

- |doxygen|_ or higher.
- |python|_ or higher.
- The following python packages, that may be installed using `PIP 
  <http://www.pip-installer.org>`_ :

  - sphinx

The compiler suites available vary per platform. We recommend the following 
setup:

.. admonition:: Windows
   :class: platform-specific win

   - The following components of *Windows SDK 7.0 SP1*:

     - Compilers
     - Headers
     - Win32 development tools

     Alternatively, one can install *Microsoft Visual C++ Express 2010* or a 
     more recent or complete version of Visual Studio. 
   - |cmake|_ or higher.

   We will assume that there is a command prompt available that performs the 
   necessary configuration to make the ``nmake`` executable available in the 
   path. This command prompt can be found as a batch file in 
   ``C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\bin`` for Windows
   SDK 7.0 SP1. In the start menu, look for either of the following:
    
    - 32bit: Visual Studio 2008 Command Prompt
    - 64bit: Visual Studio 2008 x64 Win64 Command Prompt

.. admonition:: Mac OS X
   :class: platform-specific mac

   - *XCode* must be installed. It can be found on the OS X installation DVDs.
     Consult the following table for the required version.

     +-----------------+-----------------+
     | OS X version    |  XCode version  |
     +=================+=================+
     | 10.5            | 3.1             |
     +-----------------+-----------------+
     | 10.6            | 3.2.5           |
     +-----------------+-----------------+
     | 10.7            | 4.2.1           |
     +-----------------+-----------------+


   - |cmakeosx|_ or higher.
     
.. admonition:: Linux
   :class: platform-specific linux

   - *g++* version 4.4.0 or higher.
   - *make* version 3.80 or higher.
   - |cmake|_ or higher.

   We recommend installing these by using the package manager of your
   distribution. On, e.g., Ubuntu, these can be installed by installing
   the ``build-essential`` PPA package.

Boost
-----

.. |boost| replace:: *Boost 1.36*
.. _boost: http://www.boost.org

The mCRL2 sources use libraries from the |boost|_ collection (or a more recent
version). The build only depends on the Boost header files, and does not
link to any boost libraries.

.. admonition:: Windows
   :class: platform-specific win

   .. warning:: 
 
      Only a 32-bit binary version is freely available at the time of writing. For
      64-bit versions, :doc:`compile the Boost libraries yourself <boost>`.

   To install a binary version of *Boost*, follow the steps below.

   - Download *BoostPro* from http://www.boostpro.com/download/
   - Install file after downloading.
   - Install all header files.

.. _osx-boost:

.. admonition:: Mac OS X
   :class: platform-specific mac

   .. warning:: 
 
      Only a 32-bit version seems to be available at the time of writing. For
      64-bit versions, :doc:`compile the Boost libraries yourself <boost>`.

   Boost can be installed using MacPorts by doing the following:

   - First go to http://www.macports.org/.
   - In the left menu bar select "Available Downloads" and download the 
     appropriate version.
   - Install the downloaded image.
   - After installing open a terminal and execute the following to test and 
     update MacPort::

       sudo port selfupdate
  
     Note that ``port`` is usually installed in ``/opt/local/bin``. 
   - To install *Boost*, execute::

       sudo port install boost
   
.. admonition:: Linux
   :class: platform-specific linux

   The easiest way to install Boost using the package 
   manager in your distribution. For instance, the libraries can be installed
   by installing the ``libboost-*-dev`` PPA packages when you are using
   Ubuntu.

   You can also :doc:`compile the Boost libraries yourself <boost>`.

.. _build_prerequisites_qt:

QT
--

.. |qt| replace:: *QT 4*
.. _qt: http://qt-project.org

The mCRL2 toolset requires |qt|_ for compilation of the graphical tools 
(:ref:`tool-diagraphica`, :ref:`tool-ltsgraph`, :ref:`tool-ltsview`,
:ref:`tool-lpsxsim`, :ref:`tool-mcrl2-gui`). 

.. admonition:: Windows
   :class: platform-specific win
   
   When building for the Windows SDK, it is recommended to install QT using the
   following steps:
   
   For a 32-bit version, binaries are available:
   
     - Download the Qt libraries for Windows for your compiler version from http://qt-project.org/downloads.
       For the Windows 7.1 SDK you need to select the Qt libraries for Windows (VS 2010).
     - Follow the installation instructions.
     
   For a 64-bit version, perform the following steps (taken from http://qt-project.org/doc/qt-4.8/install-win.html):
   
     - Download the Qt libraries source package (.zip) from http://qt-project.org//downloads.
     - Extract the files, e.g. to ``C:\Qt\4.8.2``
     - Add ``C:\Qt\4.8.2\bin`` to your ``PATH`` environment variable.
     - Open a *Visual Studio* command prompt, and type the following (to support
       both the Debug and the Release build)::
     
         C:
         cd /D C:\Qt\4.8.2
         configure -opensource -fast -debug-and-release -no-qt3support -no-dsp -no-vcproj  
       
       You need to accept the licence by typing ``y``.
       Qt can now be compiled by typing::
       
         nmake

       .. warning::

          A bug in 64-bit MSVC is causing a problem in the release build of the
          Qt libraries. This can be circumvented by installing the fix of Microsoft
          `KB2280741 <http://support.microsoft.com/kb/2280741>`_.
   
   When installed correctly, and the binary directory for Qt has been added to
   your ``PATH``, CMake should be able to automatically find your 
   installation.
   
.. admonition:: Mac OS X
   :class: platform-specific mac

   QT can be installed using MacPorts by doing the following:

   - First go to http://www.macports.org/.
   - In the left menu bar select "Available Downloads" and download the 
     appropriate version.
   - Install the downloaded image.
   - After installing open a terminal and execute the following to test and 
     update MacPort::

       sudo port selfupdate
  
     Note that ``port`` is usually installed in ``/opt/local/bin``. 
   - To install *QT*, execute::

       sudo port install qt5-mac 
   
.. admonition:: Linux
   :class: platform-specific linux
   
   Binary development versions are available in the package manager in most 
   distributions (for instance the ``libqt5-dev`` PPA package in Ubuntu).

   On Linux it is also required to install OpenGL related development packages.
   The exact package to be installed depends on your distribution. For Ubuntu
   these are e.g. ``libgl1-mesa-dev`` and ``libglu1-mesa-dev``.

