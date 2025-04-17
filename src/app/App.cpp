#include <app/App.h>
#include <ui/UIManager.h>
#include <config/Config.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <Windows.h>

App::App(HINSTANCE hInstance, int nCmdShow)
    : hInstance_(hInstance), nCmdShow_(nCmdShow) {
}

App::~App() = default;

bool App::Init() {
    if (!glfwInit()) return false;
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    #ifdef _WIN32
        SetProcessDPIAware();
    #endif
    return InitWindow() && InitImGui();
}

bool App::InitWindow() {
    window_ = glfwCreateWindow(
        Config::WindowWidth,
        Config::WindowHeight,
        Config::WindowTitle,
        nullptr, nullptr
    );
    if (!window_) {
        glfwTerminate();
        return false;
    }
    UIManager::SetupWindowIcons(hInstance_, window_);
    glfwSetWindowAttrib(window_, GLFW_FLOATING, GLFW_TRUE);
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);
    return true;
}

bool App::InitImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    UIManager::LoadFonts(io);
    UIManager::ApplyTheme();
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    return true;
}

void App::Run() {
    UIManager ui(window_);
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        ui.NewFrame();
        ui.Render();
        glfwSwapBuffers(window_);
    }
    Cleanup();
}

void App::Cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window_);
    glfwTerminate();
}