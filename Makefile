VULKAN_SDK_PATH = /home/meith/VulkanSDK/1.0.65.0/x86_64

CC = gcc
CFLAGS = -std=c11	-g	-ggdb	-Wall	-Werror	-I$(VULKAN_SDK_PATH)/include	`pkg-config --cflags glfw3`

LD = gcc
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib	`pkg-config --static --libs glfw3`	-lvulkan

all: main

main: main.o
	$(LD)	$^	-o	$@	$(LDFLAGS)

%o: %.c	%.h
	$(CC)	$(CFLAGS)	-c	$<	-o	$@

clean:
	rm	-f	*o	main

.PHONY: clean
