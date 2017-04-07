all:
	g++ test.cpp ../../apl-server/message.cpp -o test -std=c++11 -g -I../../3rdparty/include/ -I../../apl-server -L../../3rdparty/lib -L../../3rdparty/lib/x86_64-linux-gnu -L/usr/local/lib -Wl,-rpath ../../3rdparty/lib/x86_64-linux-gnu -ldl -lpthread -levent -ljson-linux-0.6.0 -lboost_regex -lboost_thread -lboost_filesystem -lboost_system -lssl -lcrypto -lcurl -lrabbitmq
