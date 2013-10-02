WORKDIR = `pwd`

CC = gcc
CXX = g++
AR = ar
LD = g++
WINDRES = windres

INC = 
CFLAGS = 
RESINC = 
LIBDIR = 
LIB = 
LDFLAGS = -lstdc++

INC_RELEASE =  $(INC) -I. -Iext
CFLAGS_RELEASE =  $(CFLAGS) -O2 -pedantic-errors -pedantic -std=c++11 -Wfatal-errors -Wextra -Wall
RESINC_RELEASE =  $(RESINC)
RCFLAGS_RELEASE =  $(RCFLAGS)
LIBDIR_RELEASE =  $(LIBDIR)
LIB_RELEASE = $(LIB)
LDFLAGS_RELEASE =  $(LDFLAGS) -s
OBJDIR_RELEASE = obj/Release
DEP_RELEASE = 
OUT_RELEASE = bin/Release/zeitkit

OBJ_RELEASE = $(OBJDIR_RELEASE)/ext/stream.o $(OBJDIR_RELEASE)/ext/singledocparser.o $(OBJDIR_RELEASE)/ext/simplekey.o $(OBJDIR_RELEASE)/ext/scantoken.o $(OBJDIR_RELEASE)/ext/scantag.o $(OBJDIR_RELEASE)/ext/scanscalar.o $(OBJDIR_RELEASE)/ext/scanner.o $(OBJDIR_RELEASE)/ext/regex.o $(OBJDIR_RELEASE)/ext/parser.o $(OBJDIR_RELEASE)/utils/parser.o $(OBJDIR_RELEASE)/utils/input.o $(OBJDIR_RELEASE)/utils/http.o $(OBJDIR_RELEASE)/utils/encode.o $(OBJDIR_RELEASE)/utils/checksum.o $(OBJDIR_RELEASE)/utils/time.o $(OBJDIR_RELEASE)/main.o $(OBJDIR_RELEASE)/ext/tag.o $(OBJDIR_RELEASE)/ext/emitterstate.o $(OBJDIR_RELEASE)/ext/emitter.o $(OBJDIR_RELEASE)/ext/emitfromevents.o $(OBJDIR_RELEASE)/ext/emit.o $(OBJDIR_RELEASE)/ext/directives.o $(OBJDIR_RELEASE)/ext/convert.o $(OBJDIR_RELEASE)/ext/block_allocator.o $(OBJDIR_RELEASE)/ext/binary.o $(OBJDIR_RELEASE)/core/zeitkit.o $(OBJDIR_RELEASE)/core/worklog.o $(OBJDIR_RELEASE)/core/client.o $(OBJDIR_RELEASE)/ext/parse.o $(OBJDIR_RELEASE)/ext/ostream_wrapper.o $(OBJDIR_RELEASE)/ext/null.o $(OBJDIR_RELEASE)/ext/nodeevents.o $(OBJDIR_RELEASE)/ext/nodebuilder.o $(OBJDIR_RELEASE)/ext/node_data.o $(OBJDIR_RELEASE)/ext/node.o $(OBJDIR_RELEASE)/ext/memory.o $(OBJDIR_RELEASE)/ext/json.o $(OBJDIR_RELEASE)/ext/happyhttp.o $(OBJDIR_RELEASE)/ext/exp.o $(OBJDIR_RELEASE)/ext/emitterutils.o

all: release

clean: clean_release

before_release: 
	test -d bin/Release || mkdir -p bin/Release
	test -d $(OBJDIR_RELEASE)/ext || mkdir -p $(OBJDIR_RELEASE)/ext
	test -d $(OBJDIR_RELEASE)/utils || mkdir -p $(OBJDIR_RELEASE)/utils
	test -d $(OBJDIR_RELEASE) || mkdir -p $(OBJDIR_RELEASE)
	test -d $(OBJDIR_RELEASE)/core || mkdir -p $(OBJDIR_RELEASE)/core

after_release: 

release: before_release out_release after_release

out_release: $(OBJ_RELEASE) $(DEP_RELEASE)
	$(LD) $(LDFLAGS_RELEASE) $(LIBDIR_RELEASE) $(OBJ_RELEASE) $(LIB_RELEASE) -o $(OUT_RELEASE)

$(OBJDIR_RELEASE)/ext/stream.o: ext/stream.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/stream.cpp -o $(OBJDIR_RELEASE)/ext/stream.o

$(OBJDIR_RELEASE)/ext/singledocparser.o: ext/singledocparser.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/singledocparser.cpp -o $(OBJDIR_RELEASE)/ext/singledocparser.o

$(OBJDIR_RELEASE)/ext/simplekey.o: ext/simplekey.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/simplekey.cpp -o $(OBJDIR_RELEASE)/ext/simplekey.o

$(OBJDIR_RELEASE)/ext/scantoken.o: ext/scantoken.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/scantoken.cpp -o $(OBJDIR_RELEASE)/ext/scantoken.o

$(OBJDIR_RELEASE)/ext/scantag.o: ext/scantag.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/scantag.cpp -o $(OBJDIR_RELEASE)/ext/scantag.o

$(OBJDIR_RELEASE)/ext/scanscalar.o: ext/scanscalar.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/scanscalar.cpp -o $(OBJDIR_RELEASE)/ext/scanscalar.o

$(OBJDIR_RELEASE)/ext/scanner.o: ext/scanner.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/scanner.cpp -o $(OBJDIR_RELEASE)/ext/scanner.o

$(OBJDIR_RELEASE)/ext/regex.o: ext/regex.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/regex.cpp -o $(OBJDIR_RELEASE)/ext/regex.o

$(OBJDIR_RELEASE)/ext/parser.o: ext/parser.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/parser.cpp -o $(OBJDIR_RELEASE)/ext/parser.o

$(OBJDIR_RELEASE)/utils/parser.o: utils/parser.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c utils/parser.cpp -o $(OBJDIR_RELEASE)/utils/parser.o

$(OBJDIR_RELEASE)/utils/input.o: utils/input.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c utils/input.cpp -o $(OBJDIR_RELEASE)/utils/input.o

$(OBJDIR_RELEASE)/utils/http.o: utils/http.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c utils/http.cpp -o $(OBJDIR_RELEASE)/utils/http.o

$(OBJDIR_RELEASE)/utils/encode.o: utils/encode.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c utils/encode.cpp -o $(OBJDIR_RELEASE)/utils/encode.o

$(OBJDIR_RELEASE)/utils/checksum.o: utils/checksum.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c utils/checksum.cpp -o $(OBJDIR_RELEASE)/utils/checksum.o

$(OBJDIR_RELEASE)/utils/time.o: utils/time.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c utils/time.cpp -o $(OBJDIR_RELEASE)/utils/time.o

$(OBJDIR_RELEASE)/main.o: main.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c main.cpp -o $(OBJDIR_RELEASE)/main.o

$(OBJDIR_RELEASE)/ext/tag.o: ext/tag.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/tag.cpp -o $(OBJDIR_RELEASE)/ext/tag.o

$(OBJDIR_RELEASE)/ext/emitterstate.o: ext/emitterstate.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/emitterstate.cpp -o $(OBJDIR_RELEASE)/ext/emitterstate.o

$(OBJDIR_RELEASE)/ext/emitter.o: ext/emitter.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/emitter.cpp -o $(OBJDIR_RELEASE)/ext/emitter.o

$(OBJDIR_RELEASE)/ext/emitfromevents.o: ext/emitfromevents.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/emitfromevents.cpp -o $(OBJDIR_RELEASE)/ext/emitfromevents.o

$(OBJDIR_RELEASE)/ext/emit.o: ext/emit.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/emit.cpp -o $(OBJDIR_RELEASE)/ext/emit.o

$(OBJDIR_RELEASE)/ext/directives.o: ext/directives.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/directives.cpp -o $(OBJDIR_RELEASE)/ext/directives.o

$(OBJDIR_RELEASE)/ext/convert.o: ext/convert.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/convert.cpp -o $(OBJDIR_RELEASE)/ext/convert.o

$(OBJDIR_RELEASE)/ext/block_allocator.o: ext/block_allocator.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/block_allocator.cpp -o $(OBJDIR_RELEASE)/ext/block_allocator.o

$(OBJDIR_RELEASE)/ext/binary.o: ext/binary.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/binary.cpp -o $(OBJDIR_RELEASE)/ext/binary.o

$(OBJDIR_RELEASE)/core/zeitkit.o: core/zeitkit.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c core/zeitkit.cpp -o $(OBJDIR_RELEASE)/core/zeitkit.o

$(OBJDIR_RELEASE)/core/worklog.o: core/worklog.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c core/worklog.cpp -o $(OBJDIR_RELEASE)/core/worklog.o

$(OBJDIR_RELEASE)/core/client.o: core/client.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c core/client.cpp -o $(OBJDIR_RELEASE)/core/client.o

$(OBJDIR_RELEASE)/ext/parse.o: ext/parse.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/parse.cpp -o $(OBJDIR_RELEASE)/ext/parse.o

$(OBJDIR_RELEASE)/ext/ostream_wrapper.o: ext/ostream_wrapper.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/ostream_wrapper.cpp -o $(OBJDIR_RELEASE)/ext/ostream_wrapper.o

$(OBJDIR_RELEASE)/ext/null.o: ext/null.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/null.cpp -o $(OBJDIR_RELEASE)/ext/null.o

$(OBJDIR_RELEASE)/ext/nodeevents.o: ext/nodeevents.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/nodeevents.cpp -o $(OBJDIR_RELEASE)/ext/nodeevents.o

$(OBJDIR_RELEASE)/ext/nodebuilder.o: ext/nodebuilder.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/nodebuilder.cpp -o $(OBJDIR_RELEASE)/ext/nodebuilder.o

$(OBJDIR_RELEASE)/ext/node_data.o: ext/node_data.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/node_data.cpp -o $(OBJDIR_RELEASE)/ext/node_data.o

$(OBJDIR_RELEASE)/ext/node.o: ext/node.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/node.cpp -o $(OBJDIR_RELEASE)/ext/node.o

$(OBJDIR_RELEASE)/ext/memory.o: ext/memory.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/memory.cpp -o $(OBJDIR_RELEASE)/ext/memory.o

$(OBJDIR_RELEASE)/ext/json.o: ext/json.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/json.cpp -o $(OBJDIR_RELEASE)/ext/json.o

$(OBJDIR_RELEASE)/ext/happyhttp.o: ext/happyhttp.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/happyhttp.cpp -o $(OBJDIR_RELEASE)/ext/happyhttp.o

$(OBJDIR_RELEASE)/ext/exp.o: ext/exp.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/exp.cpp -o $(OBJDIR_RELEASE)/ext/exp.o

$(OBJDIR_RELEASE)/ext/emitterutils.o: ext/emitterutils.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ext/emitterutils.cpp -o $(OBJDIR_RELEASE)/ext/emitterutils.o

clean_release: 
	rm -f $(OBJ_RELEASE) $(OUT_RELEASE)
	rm -rf bin/Release
	rm -rf $(OBJDIR_RELEASE)/ext
	rm -rf $(OBJDIR_RELEASE)/utils
	rm -rf $(OBJDIR_RELEASE)
	rm -rf $(OBJDIR_RELEASE)/core

.PHONY: before_release after_release clean_release

