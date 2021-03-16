CCARGS=-c -std=c++14 -O3
LDARGS=-g

# Files for parse tree traversal
CLIENT_SRC_FILES := $(wildcard ./*.cpp)
CLIENT_OBJ_FILES := $(patsubst ./%.cpp,build/%.o,$(CLIENT_SRC_FILES))

client : dir $(CLIENT_OBJ_FILES)
	g++ $(LDARGS) $(CLIENT_OBJ_FILES) -lform -lncurses -o server
	chmod u+x server

build/%.o : %.cpp
	g++ $(CCARGS) $< -o $@ 

dir:
	mkdir -p build

clean:
	rm -rf build
	rm server