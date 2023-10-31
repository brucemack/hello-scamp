bin/unit-test-1: build/unit-test-1.o 
		g++ -o $@ $^

unit-test-2: unit-test-2.o ClockRecoveryPLL.o
		g++ -o $@ $^

unit-test-3: unit-test-3.o Symbol6.o CodeWord12.o CodeWord24.o Frame30.o FileModulator.o \
	Util.o TestModem.o ClockRecoveryPLL.o
		g++ -o $@ $^

build/%.o:	%.cpp
		g++ -std=c++11 -g -c $^ -o $@

clean:
	rm -f *.o
	rm -f unit-test-1
