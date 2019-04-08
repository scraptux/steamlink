//
// Created by scraptux on 06.04.19.
//

#include "Discovery.h"

void Discovery::initSocket() {
    int perm = 1;
    if ((this->sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        perror("Could not create socket!");
    if (setsockopt(this->sock, SOL_SOCKET, SO_BROADCAST, (void *) &perm, sizeof(perm)) < 0)
        perror("Could not set broadcast permission!");
    if (setsockopt(this->sock, SOL_SOCKET, SO_REUSEADDR, (void *) &perm, sizeof(perm)) < 0)
        perror("Could not make socket reusable!");
    memset(&this->addr, 0, sizeof(this->addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DISCOVERY_PORT);

    if (bind(this->sock, (struct sockaddr *) &this->addr, sizeof(this->addr)) < 0)
        perror("Could not bind socket!");
}

int Discovery::getMsgType(std::string header) {
    CMsgRemoteClientBroadcastHeader head = CMsgRemoteClientBroadcastHeader();
    head.ParseFromString(header);
    return head.msg_type();
}

void Discovery::handleStatus(std::string header, std::string body) {
    CMsgRemoteClientBroadcastHeader head = CMsgRemoteClientBroadcastHeader();
    head.ParseFromString(header);
    if (head.msg_type() != k_ERemoteClientBroadcastMsgStatus)
        return;
    CMsgRemoteClientBroadcastStatus msg = CMsgRemoteClientBroadcastStatus();
    msg.ParseFromString(body);
    printf("Hostname: %s hat geantwortet\n", msg.hostname().c_str());
}

void Discovery::sendDiscover() {
    std::string buffer; // packagebuffer
    std::string tmp; // output for protos
    Byte size[4]; // bytesize for header and body

    buffer.append((const char*)this->preheader, 8);
    // prepare header
    CMsgRemoteClientBroadcastHeader header = CMsgRemoteClientBroadcastHeader();
    header.set_client_id(this->clientId);
    header.set_msg_type(k_ERemoteClientBroadcastMsgDiscovery);
    header.SerializeToString(&tmp);
    // prepare headersize (little-endian)
    for (int i = 0; i < 4; i++)
        size[i] = tmp.size() >> (i*8) & 0xff;
    // append headersize and header
    buffer.append((const char*)size, 4);
    buffer.append(tmp);
    tmp = ""; // TODO: needed?
    //prepare body
    CMsgRemoteClientBroadcastDiscovery body = CMsgRemoteClientBroadcastDiscovery();
    body.set_seq_num(this->seqNum);
    body.SerializeToString(&tmp);
    // prepare bodysize (little-endian)
    for (int i = 0; i < 4; i++)
        size[i] = tmp.size() >> (i*8) & 0xff;
    // append bodysize and body
    buffer.append((const char*)size, 4);
    buffer.append(tmp);

    inet_aton("255.255.255.255", &this->addr.sin_addr);
    if (sendto(this->sock, buffer.c_str(), buffer.size(), 0, (struct sockaddr *) &this->addr, sizeof(this->addr))
            != buffer.size())
        perror("Could not send Discovery Package!");
}

void Discovery::receiveDiscovery() {
    inet_aton("0.0.0.0", &this->addr.sin_addr);
    char response[8192];
    int n;
    socklen_t len = sizeof(struct sockaddr);
    bool discoveryPackage;
    while (true) {
        n = recvfrom(this->sock, response, sizeof(response), MSG_WAITALL, (struct sockaddr *)&this->addr, &len);
        response[n] = '\0';
        printf("Received from %s:%d\n", inet_ntoa(this->addr.sin_addr), ntohs(this->addr.sin_port));
        discoveryPackage = true;
        for (int i = 0; i < 8; i++) {
            if ((Byte)response[i] != this->preheader[i])
                discoveryPackage = false;
        }
        if (discoveryPackage) {
            int headerlen = 0, bodylen = 0;
            for (int i = 0; i < 4; i++)
                headerlen |= response[8 + i] << (i * 8);
            for (int i = 0; i < 4; i++)
                bodylen |= response[12 + headerlen + i] << (i * 8);

            char header[headerlen];
            for (int i = 0; i < headerlen; i++)
                header[i] = response[12 + i];
            char body[bodylen];
            for (int i = 0; i < bodylen; i++)
                body[i] = response[16 + headerlen + i];

            switch (this->getMsgType(std::string(header, headerlen))) {
                case k_ERemoteClientBroadcastMsgDiscovery:
                    break;
                case k_ERemoteClientBroadcastMsgStatus:
                    this->handleStatus(std::string(header, headerlen), std::string(body, bodylen));
                    break;
                default:
                    break;
            }
        }
    }
}