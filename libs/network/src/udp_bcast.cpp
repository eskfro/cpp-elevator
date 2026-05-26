#include <network/udp_bcast.hpp>

namespace elev::network {


elev::ordersync::OrderMatrix* Peers::getOrdersAt(int elevID) {
     return &allOrders[elevID];
};




}