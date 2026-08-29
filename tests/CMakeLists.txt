# ---------------------------------------------------------------------------
# Unit tests
#
# Only the pure-logic parts of the system are covered here: the request table,
# the controller decision logic, the elevator state container, the distributed
# order state machine and the timer helper. Anything that talks to the
# simulator over a socket (hardware / network / buttons) is intentionally left
# out - those need an integration harness, not unit tests.
# ---------------------------------------------------------------------------

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
    GIT_SHALLOW TRUE
    # Prefer a system-installed GTest when one is available (keeps CI / offline
    # builds fast); otherwise fall back to fetching the pinned version above.
    FIND_PACKAGE_ARGS NAMES GTest
)

# Don't let GoogleTest's own warnings/pedantry leak into our build.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# Helper: declare a test executable, link GTest and register it with CTest.
function(elev_add_test name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE GTest::gtest_main)
    gtest_discover_tests(${name})
endfunction()

elev_add_test(test_request_table test_request_table.cpp)
target_link_libraries(test_request_table PRIVATE control)

elev_add_test(test_controller test_controller.cpp)
target_link_libraries(test_controller PRIVATE control elevator)

elev_add_test(test_elevator_state test_elevator_state.cpp)
target_link_libraries(test_elevator_state PRIVATE elevator)

elev_add_test(test_order test_order.cpp)
target_link_libraries(test_order PRIVATE ordersync)

elev_add_test(test_timer test_timer.cpp)
target_link_libraries(test_timer PRIVATE common)