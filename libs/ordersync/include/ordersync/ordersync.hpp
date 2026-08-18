#pragma once

// Libs
#include <array>
#include <common/config.hpp>
#include <common/types.hpp>


using namespace elev::common;
using namespace elev::config;

namespace elev::ordersync {

class Order {
    public:
        Order() = default;

        // Set
        void Observe(int elev_id);

        // Get
        uint64_t Version() { return version_; }
        OrderStatus Status() { return status_; }
        int AssignedId() { return assigned_id_; }
        uint32_t ObservedMask() { return observed_mask_; }
        bool ObservedBy(int elev_id);

        // State machine
        void OnUpdate(Order rcv);
        void OnRequest(int elev_id);
        void OnConfirm(int elev_id);
        void OnClear();
        void OnReset();
        void OnRevoke();

    private:
        int assigned_id_{-1};
        uint32_t observed_mask_{};
        OrderStatus status_{};
        uint64_t version_{};
};

class OrderTable {
    public:
        OrderTable() = default;

        // Set
        void Join(OrderTable rcv);

        // Get
        ordersync::Order* Order(int floor, int btn);
        std::array<std::array<bool, kButtons>, kFloors> ToBoolTable(int elev_id);

    private:
        std::array<std::array<ordersync::Order, kButtons>, kFloors> table_{};
};

} // namespace elev::ordersync