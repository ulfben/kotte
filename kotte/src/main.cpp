#include "kotte/application.hpp"

#include <cstdint>
#include <exception>

int main([[maybe_unused]] int argc, [[maybe_unused]]  char** argv){    
    try{
        constexpr std::uint64_t seed = 0x4b4f545445ULL;
        kotte::Application app{1280, 720, "Kotte", seed};
        app.run();
    } catch(const std::exception& error){
        TraceLog(LOG_ERROR, "%s", error.what());
        return 1;
    }    
    return 0;
}
