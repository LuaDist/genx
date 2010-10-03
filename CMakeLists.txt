# Copyright (C) 2007-2009 LuaDist.
# Created by Peter Kapec
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
# Please note that the package source code is licensed under its own license.

PROJECT(lgenx C)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
INCLUDE(dist.cmake)

# Build
INCLUDE_DIRECTORIES(genx)

ADD_LIBRARY(genx_lib SHARED genx/genx.c genx/charProps.c)
SET_TARGET_PROPERTIES ( genx_lib PROPERTIES OUTPUT_NAME genx CLEAN_DIRECT_OUTPUT 1 )

ADD_LUA_MODULE(genx lgenx.c)
TARGET_LINK_LIBRARIES(genx genx_lib)

# Install all files and documentation
INSTALL (TARGETS genx DESTINATION ${INSTALL_CMOD})
INSTALL (TARGETS genx_lib DESTINATION ${INSTALL_LIB})
INSTALL (FILES doc/api.html doc/index.html doc/license.html doc/manual.html doc/luadoc.css DESTINATION ${INSTALL_DOC})
INSTALL (FILES test.lua DESTINATION ${INSTALL_TEST})
