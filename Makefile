#
unit-test-1: unit-test-1.o scamp-util-1.o scamp-util-2.o Encoder.o FrameSenderImpl.o FrameReceiver.o
		g++ -o $@ $^

%.o:	%.cpp
		g++ -std=c++11 -g -c $^

clean:
	rm -f *.o
	rm -f unit-test-1
