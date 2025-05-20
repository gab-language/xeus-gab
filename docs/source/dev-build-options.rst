..  Copyright (c) 2025,    

   Distributed under the terms of the MIT license.  

   The full license is in the file LICENSE, distributed with this software.

Build and configuration
=======================

General Build Options
---------------------

Building the xeus-gab library
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``xeus-gab`` build supports the following options:

- ``XEUS_GAB_BUILD_SHARED``: Build the ``xeus-gab`` shared library. **Enabled by default**.
- ``XEUS_GAB_BUILD_STATIC``: Build the ``xeus-gab`` static library. **Enabled by default**.


- ``XEUS_GAB_USE_SHARED_XEUS``: Link with a `xeus` shared library (instead of the static library). **Enabled by default**.

Building the kernel
~~~~~~~~~~~~~~~~~~~

The package includes two options for producing a kernel: an executable ``xgab`` and a Python extension module, which is used to launch a kernel from Python.

- ``XEUS_GAB_BUILD_EXECUTABLE``: Build the ``xgab``  executable. **Enabled by default**.


If ``XEUS_GAB_USE_SHARED_XEUS_GAB`` is disabled, xgab  will be linked statically with ``xeus-gab``.

Building the Tests
~~~~~~~~~~~~~~~~~~

- ``XEUS_GAB_BUILD_TESTS ``: enables the tets  **Disabled by default**.

