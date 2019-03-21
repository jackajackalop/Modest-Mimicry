#include "Connection.hpp"
#include "Game.hpp"

#include <iostream>
#include <set>
#include <chrono>

int main(int argc, char **argv) {
	if (argc != 2) {
		std::cerr << "Usage:\n\t./server <port>" << std::endl;
		return 1;
	}

	Server server(argv[1]);

    int players = 0;
	Game state1, state2, temp;

	while (1) {
		server.poll([&](Connection *c, Connection::Event evt){
			if (evt == Connection::OnOpen) {
			} else if (evt == Connection::OnClose) {
			} else { assert(evt == Connection::OnRecv);
				if (c->recv_buffer[0] == 'h') {
					c->recv_buffer.erase(c->recv_buffer.begin(), c->recv_buffer.begin() + 1);
                    if(players == 0)
                        c->send_raw("p0", 2);
                    else if(players == 1)
                        c->send_raw("p1", 2);
                    else
                        std::cout<<"Again, this is a 2 player game"<<std::endl;
					std::cout << c << ": Got hello." << std::endl;
				} else if (c->recv_buffer[0] == 's') {
					if (c->recv_buffer.size() < 1 + sizeof(float)) {
						return; //wait for more data
					} else {
						c->recv_buffer.erase(c->recv_buffer.begin(), c->recv_buffer.begin() + 1 + sizeof(float));
					}
				}else if (c->recv_buffer[0] == 'a') {
					if (c->recv_buffer.size() < 1 + 10*sizeof(Primitive)) {
						return; //wait for more data
					} else {
                        memcpy(&temp.primitives, c->recv_buffer.data()+1,
                                10*sizeof(Primitive));
						c->recv_buffer.erase(c->recv_buffer.begin(),
                                c->recv_buffer.begin()+1+ 10*sizeof(Primitive));
                        c->send_raw("a", 1);
                        if(c->ID == 0){
                            state1 = temp;
                            c->send_raw(&state2.primitives,
                                    10*sizeof(Primitive));
                        }else{
                            state2 = temp;
                            c->send_raw(&state1.primitives,
                                    10*sizeof(Primitive));
                        }
					}
				}

			}
		}, 0.01);
		//every second or so, dump the current paddle position:
		static auto then = std::chrono::steady_clock::now();
		auto now = std::chrono::steady_clock::now();
		if (now > then + std::chrono::seconds(1)) {
			then = now;
		}
	}
}
