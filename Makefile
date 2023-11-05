#---------------------------------------------------------------------
# executables
#---------------------------------------------------------------------
MD := mkdir
RM := rm

test:	bin/unit-test-1 bin/unit-test-2 bin/unit-test-3
	bin/unit-test-1
	bin/unit-test-2
	bin/unit-test-3

bin/unit-test-1: build/unit-test-1.o build/SimpleFFT.o build/TestModem2.o build/Frame30.o \
	build/CodeWord12.o build/CodeWord24.o build/Symbol6.o
	g++ -o $@ $^

bin/unit-test-2: build/unit-test-2.o build/ClockRecoveryPLL.o
	g++ -o $@ $^

bin/unit-test-3: build/unit-test-3.o build/Symbol6.o build/CodeWord12.o build/CodeWord24.o \
	build/Frame30.o build/FileModulator.o build/Util.o build/TestModem.o build/ClockRecoveryPLL.o
	g++ -o $@ $^

bin/unit-test-4: build/unit-test-4.o 
	g++ -o $@ -L/usr/local/lib $^ -lkissfft-float 

bin/unit-test-4f: build/unit-test-4f.o 
	g++ -o $@ -L/usr/local/lib $^ -lkissfft-int16_t 

bin/unit-test-5: build/unit-test-5.o 
	g++ -o $@ -L/usr/local/lib $^ -lfixmath 

bin/unit-test-6: build/unit-test-6.o
	g++ -o $@ $^ 

build/%.o:	%.cpp
	g++ -std=c++11 -g -c $^ -o $@

setup:
	mkdir -p build bin

clean:
	$(RM) -rf build bin
