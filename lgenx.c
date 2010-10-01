#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include "genx.h"

#define LGENX_DOCUMENT "genx document"
#define LGENX_LIBRARY "genx"

typedef struct {
    genxWriter writer;
    
    lua_State *L; // lua state at last library call
    int sender_sendref;
    int sender_flushref;
    
    int fileref;
} document;

document *newdoc(lua_State *L) {
    document *doc = (document *)lua_newuserdata(L, sizeof(document));
    doc->writer = NULL;
    doc->L = L;
    doc->sender_sendref = LUA_REFNIL;
    doc->sender_flushref = LUA_REFNIL;
    doc->fileref = LUA_REFNIL;
    
    // set the new writer's identifying metatable
    luaL_getmetatable(L, LGENX_DOCUMENT);
    lua_setmetatable(L, -2);
    
    // create a new genx writer with the document as userdata
    doc->writer = genxNew(NULL, NULL, (void*)doc);
    
    return doc;
}

document *checkdoc(lua_State *L, int index) {
    void *ud = luaL_checkudata(L, index, LGENX_DOCUMENT);
    luaL_argcheck(L, ud != NULL, index, "genx document expected");
    ((document *)ud)->L = L; // update the current Lua state
    return (document *)ud;
}

FILE **checkfile(lua_State *L, int index) {
    void *ud = luaL_checkudata(L, index, LUA_FILEHANDLE);
    luaL_argcheck(L, ud != NULL, index, "file handle expected");
    return (FILE **)ud;
}

static void handleError(lua_State *L, genxWriter writer, genxStatus status) {
    if (status)
        luaL_error(L, genxLastErrorMessage(writer));
}

genxStatus sender_send(void *userData, constUtf8 s) {
    document *doc = (document *)userData; // the userdata is the document
    lua_State *L = doc->L;
    
    lua_rawgeti(L, LUA_REGISTRYINDEX, doc->sender_sendref); // push send func
    lua_pushstring(L, (char *)s); // argument: string to send
    lua_call(L, 1, 0);
    return GENX_SUCCESS;
}

genxStatus sender_sendBounded(void *userData, constUtf8 start, constUtf8 end) {
    document *doc = (document *)userData;
    lua_State *L = doc->L;
    
    lua_rawgeti(L, LUA_REGISTRYINDEX, doc->sender_sendref);
    lua_pushlstring(L, (char *)start, (end-start));
    lua_call(L, 1, 0);
    return GENX_SUCCESS;
}

genxStatus sender_flush(void *userData) {
    document *doc = (document *)userData;
    lua_State *L = doc->L;
    
    // the flush function is optional, so make sure we have it
    if (doc->sender_flushref != LUA_REFNIL) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, doc->sender_flushref);
        lua_call(L, 0, 0);
    }
    return GENX_SUCCESS;
}

static genxSender sender = {sender_send, sender_sendBounded, sender_flush};



static int lgenx_new(lua_State *L) {
    document *doc;
    genxStatus s;
    
    if (lua_isfunction(L, 1)) {
        // sender initialization
        int sendref = LUA_REFNIL;
        int flushref = LUA_REFNIL;
        
        // hold references to the sender functions
        // first, send:
        lua_pushvalue(L, 1);
        sendref = luaL_ref(L, LUA_REGISTRYINDEX);
        // then, flush (optional):
        lua_pushvalue(L, 2);
        if (lua_isfunction(L, -1))
            flushref = luaL_ref(L, LUA_REGISTRYINDEX);
        else
            lua_pop(L, 1);
        
        doc = newdoc(L);
        doc->sender_sendref = sendref;
        doc->sender_flushref = flushref;
        s = genxStartDocSender(doc->writer, &sender);
    
    } else if (lua_isuserdata(L, 1)) {
        // file initializaiton
        FILE **fh = checkfile(L, 1);
        doc = newdoc(L);
        
        // hold a reference to the file object so it doesn't get collected
        lua_pushvalue(L, 1);
        doc->fileref = luaL_ref(L, LUA_REGISTRYINDEX);
        
        s = genxStartDocFile(doc->writer, *fh);
    
    } else {
        // incorrect invocation
        luaL_error(L, "new() must be invoked with functions or a file");
    }
    
    handleError(L, doc->writer, s);
    return 1;
}

static int lgenx_comment(lua_State *L) {
    document *doc = checkdoc(L, 1);
    const char *text = luaL_checkstring(L, 2);
    genxStatus s = genxComment(doc->writer, (constUtf8)text);
    handleError(L, doc->writer, s);
    return 0;
}

static int lgenx_startElement(lua_State *L) {
    document *doc = checkdoc(L, 1);
    const char *name = luaL_checkstring(L, 2);
    genxStatus s = genxStartElementLiteral(doc->writer, NULL, (constUtf8)name);
    handleError(L, doc->writer, s);
    return 0;
}

static int lgenx_endElement(lua_State *L) {
    document *doc = checkdoc(L, 1);
    genxStatus s = genxEndElement(doc->writer);
    handleError(L, doc->writer, s);
    return 0;
}

static int lgenx_attribute(lua_State *L) {
    document *doc = checkdoc(L, 1);
    const char *key = luaL_checkstring(L, 2);
    const char *value = luaL_checkstring(L, 3);
    genxStatus s = genxAddAttributeLiteral(doc->writer,
                                           NULL,
                                           (constUtf8)key,
                                           (constUtf8)value);
    handleError(L, doc->writer, s);
    return 0;
}

static int lgenx_text(lua_State *L) {
    document *doc = checkdoc(L, 1);
    const char *text = luaL_checkstring(L, 2);
    genxStatus s = genxAddText(doc->writer, (constUtf8)text);
    handleError(L, doc->writer, s);
    return 0;
}

static int lgenx_close(lua_State *L) {
    document *doc = checkdoc(L, 1);
    
    if (doc->writer) { // not closed yet
        // finish document
        genxStatus s = genxEndDocument(doc->writer);
        handleError(L, doc->writer, s);
    
        // release writer
        genxDispose(doc->writer);
        doc->writer = NULL; // mark as closed
    }
    
    // release references, if present
    if (doc->sender_sendref != LUA_REFNIL) {
        luaL_unref(L, LUA_REGISTRYINDEX, doc->sender_sendref);
        doc->sender_sendref = LUA_REFNIL;
    }
    if (doc->sender_flushref != LUA_REFNIL) {
        luaL_unref(L, LUA_REGISTRYINDEX, doc->sender_flushref);
        doc->sender_flushref = LUA_REFNIL;
    }
    if (doc->fileref != LUA_REFNIL) {
        luaL_unref(L, LUA_REGISTRYINDEX, doc->fileref);
        doc->fileref = LUA_REFNIL;
    }
        
    return 0;
}



static const struct luaL_reg lgenx[] = {
    {"new", lgenx_new},
    {NULL, NULL}
};

static const struct luaL_reg lgenx_document[] = {
    {"comment", lgenx_comment},
    {"startElement", lgenx_startElement},
    {"endElement", lgenx_endElement},
    {"attribute", lgenx_attribute},
    {"text", lgenx_text},
    {"close", lgenx_close},
    {"__gc", lgenx_close},
    {NULL, NULL}
};

int luaopen_genx(lua_State *L) {
    /* metatable for document objects */
    luaL_newmetatable(L, LGENX_DOCUMENT);
    /* __index is itself */
    lua_pushliteral(L, "__index");
    lua_pushvalue(L, -2);
    lua_rawset(L, -3);
    /* add methods to metatable */
    luaL_openlib(L, NULL, lgenx_document, 0);
    
    /* register library */
    luaL_openlib(L, LGENX_LIBRARY, lgenx, 0);
    
    return 1;
}
