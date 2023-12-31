
OBJS := build/Symbol6.o build/CodeWord12.o build/CodeWord24.o \
	build/Frame30.o build/FileModulator.o build/Util.o build/TestModem.o \
	build/TestModem2.o build/ClockRecoveryPLL.o  build/ClockRecoveryDLL.o \
	build/fixed_math.o build/fixed_fft.o \
	build/Demodulator.o build/TestDemodulatorListener.o

test:	bin/unit-test-1 bin/unit-test-2 bin/unit-test-3 bin/unit-test-7a
	bin/unit-test-1
	bin/unit-test-2
	bin/unit-test-3
	bin/unit-test-7a

bin/unit-test-1: build/unit-test-1.o build/SimpleFFT.o build/TestModem2.o build/Frame30.o \
	build/CodeWord12.o build/CodeWord24.o build/Symbol6.o
	g++ -o $@ $^

bin/unit-test-2: build/unit-test-2.o build/ClockRecoveryPLL.o
	g++ -o $@ $^

bin/unit-test-3: build/unit-test-3.o $(OBJS)
	g++ -o $@ $^

bin/unit-test-4: build/unit-test-4.o $(OBJS)
	g++ -o $@ -L/usr/local/lib $^ 

bin/unit-test-5: build/unit-test-5.o 
	g++ -o $@ -L/usr/local/lib $^ 

bin/unit-test-6: build/unit-test-6.o $(OBJS)
	g++ -o $@ -L/usr/local/lib $^ 

bin/unit-test-7a: build/unit-test-7a.o $(OBJS)
	g++ -o $@ -L/usr/local/lib $^ 

bin/unit-test-9: build/unit-test-9.o $(OBJS)
	g++ -o $@ -L/usr/local/lib $^ 

bin/unit-test-10: build/unit-test-10.o $(OBJS)
	g++ -o $@ -L/usr/local/lib $^ 

build/%.o:	%.cpp
	g++ -std=c++11 -fstack-protector-all -g -c $^ -o $@

setup:
	mkdir -p build bin

clean:
	rm -rf build bin
