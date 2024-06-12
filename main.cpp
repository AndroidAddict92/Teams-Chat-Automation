#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "Theme.h"
#include "Font.h"
#include "Resource.h"
#include <sstream>
#include <iomanip>
#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>

// draw gradient
void DrawGradientRect(ImDrawList* draw_list, ImVec2 a, ImVec2 b, ImU32 col_a, ImU32 col_b)
{
    draw_list->AddRectFilledMultiColor(a, b, col_a, col_a, col_b, col_b);
}

// encode url for browser
std::string urlEncode(const std::string& value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (char c : value) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        }
        else {
            escaped << '%' << std::setw(2) << int((unsigned char)c);
        }
    }

    return escaped.str();
}

std::string generateURL(const std::string& csNumber, const std::string& siteName, const std::string& centerType, const std::vector<std::string>& selectedEmails, const std::string& additionalEmails) {
    std::string url = "https://teams.microsoft.com/l/chat/0/0?users=";

    // add commas to emails
    for (size_t i = 0; i < selectedEmails.size(); ++i) {
        url += selectedEmails[i];
        if (i != selectedEmails.size() - 1 || !additionalEmails.empty()) {
            url += ",";
        }
    }

    // append additional emails
    url += additionalEmails;

    /*append rest of url with proper encoding
    modify the below code to restructure your group chat title... csNumber, centerType, siteName can either be removed or changed to fit your needs
    if anything is removed, be sure you correct the topicName string to format your group title correctly
    the below example would append &topicName=GROUPCHATTITLE%20YOURCSNUMBER%20YOURCENTERTYPE%20-%20YOURSITENAME to the url */

    std::string topicName = "GROUPCHATTITLE " + csNumber + " " + centerType + " - " + siteName;
    url += "&topicName=" + urlEncode(topicName);

    return url;
}

// open url
void openURL(const std::string& url) {
#ifdef _WIN32
    std::string command = "start \"\" \"" + url + "\"";
    system(command.c_str());
#elif __APPLE__
    std::string command = "open \"" + url + "\"";
    system(command.c_str());
#else
    std::string command = "xdg-open \"" + url + "\"";
    system(command.c_str());
#endif
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // glfw init
    if (!glfwInit())
        return -1;

    // make window size fixed
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    #ifdef _WIN32
        SetProcessDPIAware();
    #endif

    // window
    GLFWwindow* window = glfwCreateWindow(330, 505, "Teams Chat Automation", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // set the window ico
    HICON hIconBig = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHATAUTOMATION));
    HICON hIconSmall = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));
    if (hIconBig && hIconSmall)
    {
        HWND hwnd = glfwGetWin32Window(window);
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
    }

    glfwSetWindowAttrib(window, GLFW_FLOATING, GLFW_TRUE);

    // make current
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync

    // imgui init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // load default font
    io.Fonts->AddFontDefault();

    // load custom font with configs
    ImFontConfig font_cfg;
    font_cfg.GlyphExtraSpacing.x = 1.2f;
    font_cfg.OversampleH = 3;
    font_cfg.OversampleV = 1;
    font_cfg.PixelSnapH = true;
    Custom_Font = io.Fonts->AddFontFromMemoryTTF(TTSquaresCondensedBold, sizeof(TTSquaresCondensedBold), 15.0f, &font_cfg);
    if (Custom_Font == nullptr) {
        std::cerr << "Failed to load custom font!" << std::endl;
        return -1;
    }
    std::cout << "Custom font loaded successfully." << std::endl;

    // apply theme
    Theme2();

    // imgui style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 5.0f;
    style.GrabRounding = 5.0f;
    style.FrameBorderSize = 0.5f;
    style.ChildBorderSize = 0.5f;
    style.WindowBorderSize = 0.2f;

    // border color
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.88f, 0.88f, 0.88f, 0.88f));

    // platform/render backend
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // define core users. add more if needed. these are typically the users that are in every chat you create
    std::vector<std::string> predefinedUsers = { "user1@domain.com", "user2@domain.com", "user3@domain.com" };
    std::vector<const char*> predefinedUserNames = { "ExampleName1", "ExampleName2", "ExampleName3" };
    bool userSelected[3] = { false, false, false };
    char csNumber[128] = "";
    char siteName[128] = "";
    // you can change centerTypes to fit your location types in the group title or remove completely
    const char* centerTypes[] = { "AC", "FC", "RATM" };
    int currentCenterTypeIndex = 0;

    // email buffer
    std::vector<std::string> addedEmails;
    char emailBuffer[128] = "";

    bool showPopup = false;
    std::string errorMessage = "";

    while (!glfwWindowShouldClose(window))
    {
        // handle events
        glfwPollEvents();

        // start frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // push the font
        ImGui::PushFont(Custom_Font);

        // get console window size
        int windowWidth, windowHeight;
        glfwGetWindowSize(window, &windowWidth, &windowHeight);

        // set main window size dynamic
        ImGui::SetNextWindowSize(ImVec2((float)windowWidth, (float)windowHeight));
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);

        // fixed main window
        ImGui::Begin("GUI", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);

        // apply gradient bg
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();
        ImU32 col_a = ImGui::ColorConvertFloat4ToU32(ImVec4(0.054f, 0.255f, 0.388f, 1.0f));
        ImU32 col_b = ImGui::ColorConvertFloat4ToU32(ImVec4(0.110f, 0.073f, 0.251f, 1.0f));
        DrawGradientRect(draw_list, p, ImVec2(p.x + size.x, p.y + size.y), col_a, col_b);

        // stylize this to match buttons
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.05f, 0.05f, 0.05f, 0.54f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.19f, 0.19f, 0.19f, 0.54f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.20f, 0.22f, 0.23f, 1.00f));

        // core users are defined as people that are on almost every chat you create. users are defined on lines 155-156
        ImGui::Text("Select Core Users:");
        for (int i = 0; i < 3; ++i) {
            ImGui::Checkbox(predefinedUserNames[i], &userSelected[i]);
        }

        ImGui::PopStyleColor(3);

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        // stylize this to match buttons
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.05f, 0.05f, 0.05f, 0.54f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.19f, 0.19f, 0.19f, 0.54f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.20f, 0.22f, 0.23f, 1.00f));

        // additional emails. these are the emails of users that are not typically part of the core users. you will need to add these yourself in the GUI
        ImGui::Text("Additional Emails:");
        ImGui::InputText("##email", emailBuffer, IM_ARRAYSIZE(emailBuffer));
        ImGui::SameLine();

        ImGui::PopStyleColor(3);

        // button style
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.05f, 0.05f, 0.54f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.19f, 0.19f, 0.19f, 0.54f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.22f, 0.23f, 1.00f));

        if (ImGui::Button("Add")) {
            std::string emailStr(emailBuffer);
            if (strlen(emailBuffer) > 0 && std::find(addedEmails.begin(), addedEmails.end(), emailStr) == addedEmails.end()) {
                addedEmails.push_back(emailBuffer);
                emailBuffer[0] = '\0'; // clear buffer
            }
            else {
                errorMessage = "This email has already been added! \nPlease add a different email!";
                showPopup = true;
                ImGui::OpenPopup("Error");
            }
        }

        ImGui::PopStyleColor(3);

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        // add remove button next to added emails just in case the email is wrong and needs to be removed
        ImGui::Text("Added Emails:");

        ImGui::BeginChild("EmailsList", ImVec2(0, 100), true);

        for (size_t i = 0; i < addedEmails.size(); ++i) {
            ImGui::BulletText("%s", addedEmails[i].c_str());
            ImGui::SameLine();

            // button style
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.05f, 0.05f, 0.54f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.19f, 0.19f, 0.19f, 0.54f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.22f, 0.23f, 1.00f));

            if (ImGui::SmallButton(("X##" + std::to_string(i)).c_str())) {
                addedEmails.erase(addedEmails.begin() + i);
                --i;
            }

            ImGui::PopStyleColor(3);
        }

        ImGui::EndChild();

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        // stylize this to match buttons
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.05f, 0.05f, 0.05f, 0.54f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.19f, 0.19f, 0.19f, 0.54f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.20f, 0.22f, 0.23f, 1.00f));

        // CS number can is typically the location ID. you can change this to whatever you need for your project
        ImGui::Text("5-Digit CS Number:");
        ImGui::SetNextItemWidth(75.0f); // field size
        ImGui::InputText("##csNumber)", csNumber, IM_ARRAYSIZE(csNumber));
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        // this will be the name of the location
        ImGui::Text("Site Name:");
        ImGui::SetNextItemWidth(175.0f); // field size
        ImGui::InputText("##siteName", siteName, IM_ARRAYSIZE(siteName));

        ImGui::PopStyleColor(3);
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.05f, 0.05f, 0.05f, 0.54f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.19f, 0.19f, 0.19f, 0.54f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.20f, 0.22f, 0.23f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.05f, 0.05f, 0.54f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.19f, 0.19f, 0.19f, 0.54f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.22f, 0.23f, 1.00f));

        // this is the type of facility related to the chat
        ImGui::Text("Center Type:");
        ImGui::SetNextItemWidth(60.0f); // field size
        ImGui::Combo("##centerTypes", &currentCenterTypeIndex, centerTypes, IM_ARRAYSIZE(centerTypes));

        ImGui::PopStyleColor(6);
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        // button style
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.05f, 0.05f, 0.54f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.19f, 0.19f, 0.19f, 0.54f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.22f, 0.23f, 1.00f));

        if (ImGui::Button("Create Chat")) {
            std::vector<std::string> selectedEmails;
            for (int i = 0; i < 3; ++i) {
                if (userSelected[i]) {
                    selectedEmails.push_back(predefinedUsers[i]);
                }
            }
            // combine all
            selectedEmails.insert(selectedEmails.end(), addedEmails.begin(), addedEmails.end());

            std::string additionalEmailsStr;
            for (const auto& email : selectedEmails) {
                additionalEmailsStr += email + ",";
            }
            if (!additionalEmailsStr.empty()) {
                additionalEmailsStr.pop_back(); // remove trailing comma on last email addr
            }

            std::string currentURL = generateURL(csNumber, siteName, centerTypes[currentCenterTypeIndex], {}, additionalEmailsStr);

            openURL(currentURL);
        }

        ImGui::SameLine();

        if (ImGui::Button("Exit")) {
            exit(0);
        }


        ImGui::PopStyleColor(3);

        ImGui::End();

        // pop the font
        ImGui::PopFont();

        // popup for duplicate email errors
        if (showPopup) {
            ImGui::OpenPopup("Error");
        }

        if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            // draw list for the popup
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 p = ImGui::GetWindowPos();
            ImVec2 size = ImGui::GetWindowSize();
            ImU32 col_a = ImGui::ColorConvertFloat4ToU32(ImVec4(0.054f, 0.255f, 0.388f, 1.0f));
            ImU32 col_b = ImGui::ColorConvertFloat4ToU32(ImVec4(0.110f, 0.073f, 0.251f, 1.0f));
            DrawGradientRect(draw_list, p, ImVec2(p.x + size.x, p.y + size.y), col_a, col_b);

            // popup styles
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.88f, 0.88f, 0.88f, 0.88f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.5f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.5f);

            // display error message popup
            ImGui::Text("%s", errorMessage.c_str());
            if (ImGui::Button("OK")) {
                ImGui::CloseCurrentPopup();
                showPopup = false;
            }
            ImGui::PopStyleColor(1);
            ImGui::PopStyleVar(5);
            ImGui::EndPopup();
        }

        // render
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }


    // clean
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
