#include "kotte.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]]  char** argv){
    kotte::Window w{1280, 720, "kotte"};        
    while(!w.should_close()){
        kotte::DrawScopeGuard dg{Color{0x11, 0x22, 0x33, 0xFF}};
        //render code here. :) 
    }
    return 0;
}
