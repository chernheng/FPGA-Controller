CCARGS=-c -std=c++14 -O3 -g
LDARGS=-g

# Files for parse tree traversal
CLIENT_SRC_FILES := $(wildcard ../include/*.cpp)
CLIENT_OBJ_FILES := $(patsubst ../include/%.cpp,build/%.o,$(CLIENT_SRC_FILES))

old_client : dir $(CLIENT_OBJ_FILES)
	g++ $(LDARGS) $(CLIENT_OBJ_FILES) reference/clientmain.cpp -lform -lncurses -o $@
	chmod u+x $@

client : dir $(CLIENT_OBJ_FILES)
	g++ $(LDARGS) $(CLIENT_OBJ_FILES) clientmain.cpp -lform -lncurses -o $@
	chmod u+x $@

network : dir $(CLIENT_OBJ_FILES)
	g++ $(LDARGS) $(CLIENT_OBJ_FILES) client.cpp -lform -lncurses -o $@
	chmod u+x $@

build/%.o : ../include/%.cpp
	g++ $(CCARGS) $< -o $@ 

dir:
	mkdir -p build

clean:
	rm -rf build
	rm -f client
	rm -f old_client
	rm -f network
