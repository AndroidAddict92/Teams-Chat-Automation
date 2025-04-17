#include <app/App.h>

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    App app{ hInstance, nCmdShow };
    if (!app.Init()) return -1;
    app.Run();
    return 0;
}
