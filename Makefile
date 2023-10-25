#
unit-test-1: unit-test-1.o scamp-util-1.o scamp-util-2.o Encoder.o FrameSenderImpl.o FrameReceiver.o
		g++ -o $@ $^

unit-test-2: unit-test-2.o
		g++ -o $@ $^

unit-test-3: unit-test-3.o Symbol6.o CodeWord12.o CodeWord24.o Frame30.o FileModulator.o
		g++ -o $@ $^

%.o:	%.cpp
		g++ -std=c++11 -g -c $^

clean:
	rm -f *.o
	rm -f unit-test-1
