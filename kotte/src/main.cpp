#include <stdexcept>
#include "application.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]]  char** argv){    
    try{
        kotte::Application app;
        app.run();
    } catch(const std::exception& error){
        TraceLog(LOG_ERROR, "%s", error.what());
        return 1;
    }    
    return 0;
}
