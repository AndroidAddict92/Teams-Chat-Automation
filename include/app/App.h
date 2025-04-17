#pragma once
#include <Windows.h>
#include <GLFW/glfw3.h>

class App {
public:
    App(HINSTANCE hInstance, int nCmdShow);
    ~App();

    bool Init();
    void Run();

private:
    bool InitWindow();
    bool InitImGui();
    void Cleanup();

    HINSTANCE hInstance_;
    int       nCmdShow_;
    GLFWwindow* window_ = nullptr;
};