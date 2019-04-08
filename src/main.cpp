//
// Created by scraptux on 06.04.19.
//

#include "networking/Discovery.h"

int main() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    Discovery t = Discovery();
    t.sendDiscover();
    t.receiveDiscovery();
    return 0;
}