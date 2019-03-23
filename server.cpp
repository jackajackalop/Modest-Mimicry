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
    int elapsed_time = 0;
	static auto then = std::chrono::steady_clock::now();
	Game state1, state2, temp;

	while (1) {
		server.poll([&](Connection *c, Connection::Event evt){
			if (evt == Connection::OnOpen) {
			} else if (evt == Connection::OnClose) {
			} else { assert(evt == Connection::OnRecv);
				if (c->recv_buffer[0] == 'h') {
					c->recv_buffer.erase(c->recv_buffer.begin(), c->recv_buffer.begin() + 1);
                    if(players == 0){
                        c->send_raw("p0", 2);
                        players++;
                    }else if(players == 1){
                        c->send_raw("p1", 2);
                        players++;
                    }else
                        std::cout<<"Again, this is a 2 player game"<<std::endl;
					std::cout << c << ": Got hello." << std::endl;
				} else if (c->recv_buffer[0] == 's') {
					if (c->recv_buffer.size() < 1 + sizeof(float)) {
						return; //wait for more data
					} else {
						c->recv_buffer.erase(c->recv_buffer.begin(), c->recv_buffer.begin() + 1 + sizeof(float));
					}
				}else if (c->recv_buffer[0] == 'a') {
                    c->send_raw("a", 1);
					if (c->recv_buffer.size() < 1 + sizeof(temp)) {
						return; //wait for more data
					} else {
                        memcpy(&temp, c->recv_buffer.data()+1,
                            sizeof(temp));
						c->recv_buffer.erase(c->recv_buffer.begin(),
                            c->recv_buffer.begin()+1+sizeof(temp));
                        if(c->ID == 0){
                            state1 = temp;
                            c->send_raw(&state2, sizeof(state2));
                        }else{
                            state2 = temp;
                            c->send_raw(&state1, sizeof(state1));
                        }
                        c->send_raw(&elapsed_time, sizeof(int));
                        //std::cout<<elapsed_time<<std::endl;
					}
				}
			}
		}, 0.01);
		//every second or so, dump the current paddle position:
		auto now = std::chrono::steady_clock::now();
        elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(now-then).count();
	}
}
