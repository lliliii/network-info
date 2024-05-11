#https://developer.android.com/ndk/guides/cpp-support?hl=ko
#https://stackoverflow.com/questions/55184167/clang-linking-so-library-libc-shared-so

NDK_PATH=/home/a/x/Android/Sdk/ndk/25.1.8937393
TOOLCHAIN=$(NDK_PATH)/toolchains/llvm/prebuilt/linux-x86_64/bin

export CXX=$(TOOLCHAIN)/armv7a-linux-androideabi23-clang++

export NDK_PATH
export TOOLCHAIN
export CXX

CXXFLAGS+=-static-libstdc++
DEVICE_PATH=/data/local/tmp

TARGET=network_info

X86_64_OUT=$(TARGET).x86_64.bin
ARMV7A_OUT=$(TARGET).armv7a.bin

# libpcap
LDFLAGS+=-L./lib/sysroot/lib
CFLAGS+=-I./lib/sysroot/include
LDLIBS+=-lpcap

all: x86_64  arm64_cpp push_and_run

x86_64:
	g++ -o $(X86_64_OUT) ${TARGET}.cpp ${CXXFLAGS} #$(X86_CFLAGS) $(X86_LDFLAGS) $(LDLIBS)

arm64_cpp:
	$(CXX) -o $(ARMV7A_OUT) ${TARGET}.cpp ${CXXFLAGS} $(CFLAGS) $(LDFLAGS) $(LDLIBS)

push_and_run: arm64_cpp
	adb push $(ARMV7A_OUT) $(DEVICE_PATH)
	adb shell chmod 755 $(DEVICE_PATH)/$(ARMV7A_OUT)
	adb shell su -c "$(DEVICE_PATH)/$(ARMV7A_OUT)"

clean:
	rm -f $(TARGET) *.o
