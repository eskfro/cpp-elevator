#include <network/udp_bcast.hpp>

namespace elev::network {


Peers::Peers() : numElevs{0} {}


void Peers::setClearOrders(int elevID, int floor, ButtonFlags b2c) {
     allOrders.at(elevID).setFromButtonFlags(elevID, floor, b2c);
}


void Peers::setNumElevs(int n) {
     numElevs = n;
}


int Peers::getNumElevs() {
     return numElevs;
}


elev::ordersync::OrderSlice Peers::getSliceFor(int elevID) {
     elev::ordersync::OrderMatrix localOrders = allOrders.at(elevID);
     return localOrders.getSliceAt(elevID);
}


void Peers::registerBtnPress(int elevID, int floor, BtnType btn, OrderStatus status) {
     allOrders.at(elevID).setStatusAt(elevID, floor, btn, status);
}



}