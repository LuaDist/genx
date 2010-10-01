package = "genx"
version = "0.5.0-1"
source = {
   url = "http://luaforge.net/frs/download.php/4197/genx-0.5.0.tar.gz"
}
description = {
   summary = "A library for safely generating XML.",
   detailed = [[
       You can use Genx to generate XML output, suitable for saving into
       a file or sending in a message to another computer program. It was
       written by Tim Bray. This Lua binding is by Adrian Sampson.
   ]],
   homepage = "http://genx.luaforge.net/",
   license = "BSD"
}
dependencies = {
   "lua >= 5.1"
}
build = {
    type = "builtin",
    modules = {
        genx = {
            sources = {"genx/genx.c", "genx/charProps.c", "lgenx.c"},
            incdirs = {"genx"},
        },
    }
}
