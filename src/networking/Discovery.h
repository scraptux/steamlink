//
// Created by scraptux on 06.04.19.
//

#ifndef STEAMLINK_DISCOVERY_H
#define STEAMLINK_DISCOVERY_H

#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <zconf.h>

#include "discovery.pb.h"

#define DISCOVERY_PORT 27036

class Discovery {
private:
    int sock;
    struct sockaddr_in addr;
    uint32_t seqNum = 1;
    uint64_t clientId = 16581058299777471175;
    Byte preheader[8] = {0xff, 0xff, 0xff, 0xff, 0x21, 0x4c, 0x5f, 0xa0};
    // TODO: Array for all discovered instances

    void initSocket();
    int getMsgType(std::string header);
    void handleStatus(std::string header, std::string body);

public:
    Discovery() {
        this->initSocket();
    }

    Discovery(uint64_t clientId) {
        this->clientId = clientId;
    }

    void sendDiscover();
    // TODO: void sendStatus();
    void receiveDiscovery();
};


#endif //STEAMLINK_DISCOVERY_H
