#include <chrono>
#include <cstdint>
#include <iostream>
#include <tuple>
#include <typeinfo>

#include <experimental/tuple>

#include <dbus/dbus.h>

#include "common.h"

using namespace std;
using namespace std::chrono;

int main() {
    using clock = steady_clock;

	auto connection = dbus::Connection{};

    auto start = clock::now();

	int32_t input = 42;
    int32_t output;

    for (auto i = 0; i < 10'000; ++i) {
        auto request = dbus::MethodCall(
            "de.engelmarkus.TheServer",
            "/de/engelmarkus/TheServer",
            "de.engelmarkus.TheServer",
            "getData"
        );

    	request.appendArguments(input);

    	auto reply = connection.send_with_reply_and_block(request, 1000);

    	tie(output) = reply.getArguments<int32_t>();
    }

    auto finish = clock::now();

	std::cout << "Elapsed time: " << duration_cast<microseconds>(finish - start).count() << " µs" << std::endl;
}
