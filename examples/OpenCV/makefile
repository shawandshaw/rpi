.PHONY: all clean build
BUILD_DIR = ./build
DEST_DIR = ./bin

all: build
	cd $(BUILD_DIR);make
clean:
	rm $(BUILD_DIR) -rf; rm $(DEST_DIR) -rf
build:
	mkdir $(BUILD_DIR);cd $(BUILD_DIR); cmake ..