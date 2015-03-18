
PLUGIN_NAME = sync

HEADERS = sync.h

SOURCES = sync.cpp \
          moc_sync.cpp

LIBS = 

### Do not edit below this line ###

include $(shell rtxi_plugin_config --pkgdata-dir)/Makefile.plugin_compile
