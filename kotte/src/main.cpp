#include "kotte.hpp"
#include <raylib.h>

int main([[maybe_unused]] int argc, [[maybe_unused]]  char** argv){
    
    InitWindow(920, 920, "kotte");
    InitAudioDevice();
    
    SetExitKey(KEY_ESCAPE);

    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(Color{0x11, 0x22, 0x33, 0xFF});
        DrawFPS(GetScreenWidth() - 100, 2);
        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();

    return 0;
}
