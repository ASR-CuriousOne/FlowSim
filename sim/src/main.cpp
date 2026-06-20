#include <Sim/sim.hpp>
#include <atomic>
#include <csignal>

namespace FluidSim {
std::atomic<bool> g_shutdownRequested{false};
}

void closeSigHandler(int signal) {
    if (signal == SIGINT) {
        FluidSim::g_shutdownRequested = true;
    }
}

int main() {
    FluidSim::Sim sim;

    std::signal(SIGINT, closeSigHandler);

    sim.run();
}
