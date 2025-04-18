#define GLFW_EXPOSE_NATIVE_WIN32

#include <ui/UIManager.h>
#include <config/Config.h>
#include <util/GraphicsUtils.h>
#include <url/url.h>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <Theme.h>
#include <Font.h>
#include <Resource.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_freetype.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>


UIManager::UIManager(GLFWwindow* window)
    : window_(window)
{
}

void UIManager::NewFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UIManager::Render() {
    int fb_w, fb_h;
    glfwGetFramebufferSize(window_, &fb_w, &fb_h);
    ImDrawList* bg = ImGui::GetBackgroundDrawList();
    bg->AddRectFilledMultiColor(
        ImVec2(0, 0),
        ImVec2((float)fb_w, (float)fb_h),
        Config::ColorA, Config::ColorA,
        Config::ColorB, Config::ColorB
    );

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::Begin("GUI", nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBackground
    );
    DrawMainWindow();
    ImGui::End();
    ImGui::PopStyleColor();

    DrawErrorPopup();

    ImGui::Render();

    glViewport(0, 0, fb_w, fb_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIManager::SetupWindowIcons(HINSTANCE hInst, GLFWwindow* glfwWindow) {
    HICON hIconBig = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CHATAUTOMATION));
    HICON hIconSmall = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SMALL));
    if (hIconBig && hIconSmall) {
        HWND hwnd = glfwGetWin32Window(glfwWindow);
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
    }
}

void UIManager::LoadFonts(ImGuiIO& io) {
    io.Fonts->Clear();

    ImFontConfig cfg;
    cfg.OversampleH = 4;
    cfg.OversampleV = 4;
    cfg.PixelSnapH = false;

    ImFont* segoe = io.Fonts->AddFontFromFileTTF(
        "C:/Windows/Fonts/segoeui.ttf",
        18.0f,
        &cfg,
        io.Fonts->GetGlyphRangesDefault()
    );
    if (!segoe) {
        segoe = io.Fonts->AddFontDefault();
    }
    io.Fonts->Build();
}

void UIManager::ApplyTheme() {
    Theme();
    ImGui::StyleColorsDark();
    ImGuiStyle& s = ImGui::GetStyle();
    s.FrameRounding = 5.0f;
    s.GrabRounding = 5.0f;
    s.FrameBorderSize = 0.5f;
    s.ChildBorderSize = 0.5f;
    s.WindowBorderSize = 0.2f;
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.88f, 0.88f, 0.88f, 0.88f));
}

void UIManager::DrawMainWindow() {
    ImGui::SetNextWindowSize({ (float)Config::WindowWidth, (float)Config::WindowHeight });
    ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Always);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::Begin("GUI", nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBackground
    );

    ImGui::Text("Select Core Users:");
    for (int i = 0; i < (int)Config::PredefinedUsers.size(); ++i) {
        ImGui::Checkbox(Config::PredefinedNames[i], &userSelected_[i]);
    }
    ImGui::Spacing();

    ImGui::Text("Additional Emails:");
    ImGui::InputText("##EmailInput", emailBuffer_, IM_ARRAYSIZE(emailBuffer_));
    ImGui::SameLine();
    if (ImGui::Button("Add")) {
        std::string e{ emailBuffer_ };
        if (!e.empty() &&
            std::find(addedEmails_.begin(), addedEmails_.end(), e) == addedEmails_.end())
        {
            addedEmails_.push_back(e);
            emailBuffer_[0] = '\0';
        }
        else {
            errorMessage_ = "This email has already been added!";
            showPopup_ = true;
        }
    }
    ImGui::Spacing();

    ImGui::Text("Added Emails:");
    ImGui::BeginChild("EmailsList", { 0,100 }, true);
    for (size_t i = 0; i < addedEmails_.size(); ++i) {
        ImGui::BulletText("%s", addedEmails_[i].c_str());
        ImGui::SameLine();
        if (ImGui::SmallButton(("X##" + std::to_string(i)).c_str())) {
            addedEmails_.erase(addedEmails_.begin() + i);
            --i;
        }
    }
    ImGui::EndChild();
    ImGui::Spacing();

    ImGui::Text("5-Digit CS Number:");
    ImGui::SetNextItemWidth(100.0f);
    ImGui::InputText("##CSNumberInput", csNumber_, IM_ARRAYSIZE(csNumber_));
    ImGui::Spacing();

    ImGui::Text("Site Name:");
    ImGui::SetNextItemWidth(200.0f);
    ImGui::InputText("##SiteNameInput", siteName_, IM_ARRAYSIZE(siteName_));
    ImGui::Spacing();

    ImGui::Text("Center Type:");
    ImGui::Combo("##CenterTypeCombo",
        &centerTypeIdx_,
        Config::CenterTypes.data(),
        (int)Config::CenterTypes.size());
    ImGui::Spacing();

    if (ImGui::Button("Create Chat")) {
        std::vector<std::string> recipients;
        for (int i = 0; i < (int)Config::PredefinedUsers.size(); ++i) {
            if (userSelected_[i])
                recipients.push_back(Config::PredefinedUsers[i]);
        }
        recipients.insert(
            recipients.end(),
            addedEmails_.begin(),
            addedEmails_.end()
        );

        std::cout << "Recipients:\n";
        for (auto& e : recipients)
            std::cout << "  " << e << "\n";

        auto url = Url::GenerateChatURL(
            csNumber_,
            siteName_,
            Config::CenterTypes[centerTypeIdx_],
            recipients
        );
        std::cout << "URL: " << url << "\n";
        Url::OpenURL(url);
    }
    ImGui::SameLine();
    if (ImGui::Button("Exit"))
        std::exit(0);

    ImGui::End();
    ImGui::PopStyleColor();
}

void UIManager::DrawErrorPopup() {
    if (showPopup_) ImGui::OpenPopup("Error");
    if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        auto dl = ImGui::GetWindowDrawList();
        auto p = ImGui::GetWindowPos();
        auto sz = ImGui::GetWindowSize();
        GraphicsUtil::DrawGradientRect(dl, p, { p.x + sz.x,p.y + sz.y },
            Config::ColorA, Config::ColorB);
        ImGui::TextUnformatted(errorMessage_.c_str());
        if (ImGui::Button("OK")) {
            showPopup_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}