#pragma once
#include <Windows.h>
#include <vector>
#include <string>
#include <GLFW/glfw3.h>
#include <imgui.h>

class UIManager {
public:
    explicit UIManager(GLFWwindow* window);
    void NewFrame();
    void Render();

    static void SetupWindowIcons(HINSTANCE hInst, GLFWwindow* window);
    static void LoadFonts(ImGuiIO& io);
    static void ApplyTheme();

private:
    void DrawMainWindow();
    void DrawErrorPopup();

    GLFWwindow* window_;
    bool                 userSelected_[3] = { false };
    char                 csNumber_[128] = "";
    char                 siteName_[128] = "";
    int                  centerTypeIdx_ = 0;
    char                 emailBuffer_[128] = "";
    std::vector<std::string> addedEmails_;
    std::string          errorMessage_;
    bool                 showPopup_ = false;
};