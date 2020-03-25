all: recvfile sendfile

recvfile: PacketRecvHandler.h PacketRecvHandler.cpp recvfile.cpp helper.h helper.cpp
	g++ -std=c++11 -o recvfile PacketRecvHandler.h PacketRecvHandler.cpp recvfile.cpp helper.h helper.cpp

sendfile: sendfile.cpp PacketSendHandler.cpp PacketSendHandler.h helper.h helper.cpp
	g++ -std=c++11 -o sendfile sendfile.cpp PacketSendHandler.cpp PacketSendHandler.h helper.h helper.cpp

clean:
	rm -f *.o
	rm -f *~
	rm -f core.*.*
	rm -f sendfile
	rm -f recvfile

