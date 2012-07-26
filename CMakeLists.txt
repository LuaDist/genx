# Copyright (C) 2007-2012 LuaDist.
# Created by Peter Kapec, Peter Drahoš
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
# Please note that the package source code is licensed under its own license.

project ( lgenx C )
cmake_minimum_required ( VERSION 2.8 )
include ( cmake/dist.cmake )
include ( lua )

# Build
include_directories ( genx )
add_library ( genx_lib genx/genx.c genx/charProps.c )
set_target_properties ( genx_lib PROPERTIES OUTPUT_NAME genx CLEAN_DIRECT_OUTPUT 
  1 )

install_library ( genx_lib )
install_lua_module ( genx lgenx.c LINK genx_lib )

install_doc ( doc/api.html doc/index.html doc/license.html doc/manual.html doc/luadoc.css )
install_test ( test.lua )
