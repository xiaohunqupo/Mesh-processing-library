MeshRoot = ..

# DEBUG_MAKE = 1

include $(MeshRoot)/make/Makefile_defs

$(call prepend,LDLIBS,$(MeshRoot)/lib$(Hw)/lib$(Hw).$(extlib) $(graphics_libs) )
includes += -I$(MeshRoot)/lib$(Hw)

# Creating a windows app (rather than console app) seems a bad idea as it disconnects from the shell
#  and does not launch subprocesses (RFile / WFile / my_system) without constructing new consoles anyways.
# Instead, I use "ShowWindow(GetConsoleWindow(), SW_HIDE);" within libHwWindows/Hw.cpp.
# LDFLAGS += -subsystem:windows -entry:mainCRTStartup

include $(MeshRoot)/make/Makefile_prog
