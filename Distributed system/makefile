CXX= g++
THR= lpthread
MAKE= make

berkeley : server_berkeley.cpp client_berkeley.cpp
	$(CXX) -o server server_berkeley.cpp -$(THR)
	$(CXX) -o client client_berkeley.cpp


cristian : server_cristian.cpp client_cristian.cpp
	$(CXX) -o server server_cristian.cpp -$(THR)
	$(CXX) -o client client_cristian.cpp

ntp : server_ntp.c client_ntp.c
	gcc server_ntp.c -o server
	gcc client_ntp.c -o client

lamport : lamport.cpp input.txt
	$(CXX) -o lamport.o lamport.cpp

clean :
	rm server client
	ls -ltr
	$(info CLEAN SUCCESSFUL!!!)

run_server :
	./server

run_client :
	./client

run_ntp_server:
	sudo ./server

run_ntp_client:
	./client 127.0.0.1

run_lamport:
	./lamport.o
