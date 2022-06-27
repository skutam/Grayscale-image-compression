FILES := $(shell find $(SOURCEDIR) -name '*.cpp')
OUT_NAME=huff_codec

all:
	g++ -std=c++17 -Werror -Wall -Wextra $(FILES) -o $(OUT_NAME)

clean:
	@rm huff_codec || true
