#pragma once
#include <windows.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "examples/example_win32_directx11/imgui_settings.h"
#include "imgui.h"
#include "Authorization/auth.hpp"
#include "Authorization/utils.hpp"
#include "Authorization/skStr.h"
#include "notify.h"
#include "DiscordRPC/Discord.h"



using namespace KynexAuth;


api KynexAuthApp(
    AppName{ skCrypt("YOUR_APP_NAME").decrypt() },       // App Name from dashboard
    OwnerID{ skCrypt("YOUR_APP_KEY").decrypt() },        // App Key (Public Owner ID)
    Version{ skCrypt("1.0").decrypt() },                 // App Version matching dashboard
    AppUrl{ skCrypt("https://kynexauth.com/api/v1/client").decrypt() } // API Endpoint
);









inline ImVec2 HubSize = ImVec2(350, 580);
HWND hwnd;
RECT rc;
static bool Hub_Login = true;
static bool Hub_Home = false;
static char username[128] = "";
static char password[128] = "";
static char license_key[128] = "";
static int login_tab = 0;
static int active_login_tab = 0;
static float login_tab_alpha = 1.0f;
void move_window() {
    ImGui::SetCursorPos(ImVec2(0, 0));
    if (ImGui::InvisibleButton("Move_detector", ImVec2(HubSize.x, HubSize.y)));
    if (ImGui::IsItemActive()) {

        GetWindowRect(hwnd, &rc);
        MoveWindow(hwnd, rc.left + ImGui::GetMouseDragDelta().x, rc.top + ImGui::GetMouseDragDelta().y, HubSize.x, HubSize.y, TRUE);
    }
}




static int iTheme = 0;

ImVec4 color_edit4 = { 0.0118f, 0.9882f, 0.9568f, 1.0f };
ImVec4 color_edit4_1 = { 1.f, 0.5f, 0.65f, 1.f };
ImVec4 color_edit4_2 = { 0.f, 1.f, 0.25f, 1.f };
ImVec4 color_edit4_3 = { 1.f, 1.f, 1.f, 1.f };

void CustomStyleColor() // ��������� ������
{
    ImGuiStyle& s = ImGui::GetStyle();
    ImGuiContext& g = *GImGui;

    s.ChildRounding = 10.f;
    s.WindowRounding = 14.f;
    s.WindowPadding = ImVec2(0, 0);

    s.Colors[ImGuiCol_Border] = ImColor(0, 0, 0, 0);
    s.Colors[ImGuiCol_Separator] = ImLerp(s.Colors[ImGuiCol_Separator], iTheme == 0 ? ImColor(28, 28, 28, 255) : iTheme == 1 ? ImColor(0, 0, 0, 43) : ImColor(28, 28, 28, 255), g.IO.DeltaTime * 10.f);
    //s.Colors[ImGuiCol_WindowBg] = ImLerp(s.Colors[ImGuiCol_WindowBg], iTheme == 0 ? color_edit4 : iTheme == 1 ? ImColor(255, 255, 255, 240) : color_edit4, g.IO.DeltaTime * 10.f);
    c::bg_decorative_rect = ImLerp(c::bg_decorative_rect, iTheme == 0 ? ImColor(255, 255, 255, 255) : iTheme == 1 ? ImColor(34, 35, 46, 255) : ImColor(255, 255, 255, 255), g.IO.DeltaTime * 10.f);

    c::child_rect = ImLerp(c::child_rect, iTheme == 0 ? ImColor(255, 255, 255, 255) : iTheme == 1 ? ImColor(34, 35, 46, 255) : ImColor(255, 255, 255, 255), g.IO.DeltaTime * 10.f);

    c::black = ImLerp(c::black, iTheme == 0 ? ImColor(255, 255, 255, 255) : iTheme == 1 ? ImColor(0, 0, 0, 255) : ImColor(255, 255,255, 255), g.IO.DeltaTime * 10.f);

    c::black_in = ImLerp(c::black_in, iTheme == 0 ? ImColor(255, 255, 255, 127) : iTheme == 1 ? ImColor(0, 0, 0, 197) : ImColor(255, 255, 255, 127), g.IO.DeltaTime * 10.f);

    c::border_child = ImLerp(c::border_child, iTheme == 0 ? ImColor(40, 42, 56, 18) : iTheme == 1 ? ImColor(255, 255, 255, 18) : ImColor(255, 255, 255, 18), g.IO.DeltaTime * 10.f);

    c::checkbox_bg = ImLerp(c::checkbox_bg, iTheme == 0 ? ImColor(25, 25, 25, 255) : iTheme == 1 ? ImColor(72, 72, 72, 255) : ImColor(25, 25, 25, 255), g.IO.DeltaTime * 10.f);

    c::bg_image = ImLerp(c::bg_image, iTheme == 0 ? ImColor(21, 21, 21, 255) : iTheme == 1 ? ImColor(255, 255, 255, 255) : ImColor(21, 21, 21, 255), g.IO.DeltaTime * 10.f);

    c::circle_checkbox = ImLerp(c::circle_checkbox, iTheme == 0 ? ImColor(51, 51, 51, 255) : iTheme == 1 ? ImColor(110, 110, 110, 255) : ImColor(51, 51, 51, 255), g.IO.DeltaTime * 10.f);
}

ImFont* Montserrat = nullptr;
ImFont* Montserrat_1 = nullptr;
ImFont* Montserrat_2 = nullptr;
int page = 0;
int sub_page = 0;
static float tab_alpha = 0.f; /* */ static float tab_add; /* */ static int active_tab = 0;

int select1 = 0;
const char* items1[5]{ "Default", "Default 1", "Default 2", "Default 3", "Default 4" };

namespace ImGui
{
    int rotation_start_index;
    void ImRotateStart()
    {
        rotation_start_index = ImGui::GetWindowDrawList()->VtxBuffer.Size;
    }

    ImVec2 ImRotationCenter()
    {
        ImVec2 l(FLT_MAX, FLT_MAX), u(-FLT_MAX, -FLT_MAX); // bounds

        const auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
        for (int i = rotation_start_index; i < buf.Size; i++)
            l = ImMin(l, buf[i].pos), u = ImMax(u, buf[i].pos);

        return ImVec2((l.x + u.x) / 2, (l.y + u.y) / 2); // or use _ClipRectStack?
    }


    void ImRotateEnd(float rad, ImVec2 center = ImRotationCenter())
    {
        float s = sin(rad), c = cos(rad);
        center = ImRotate(center, s, c) - center;

        auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
        for (int i = rotation_start_index; i < buf.Size; i++)
            buf[i].pos = ImRotate(buf[i].pos, s, c) - center;
    }
}

void Trinage_background()
{

    ImVec2 screen_size = { (float)GetSystemMetrics(SM_CXSCREEN), (float)GetSystemMetrics(SM_CYSCREEN) };

    static ImVec2 partile_pos[100];
    static ImVec2 partile_target_pos[100];
    static float partile_speed[100];
    static float partile_size[100];
    static float partile_radius[100];
    static float partile_rotate[100];

    for (int i = 1; i < 50; i++)
    {
        if (partile_pos[i].x == 0 || partile_pos[i].y == 0)
        {
            partile_pos[i].x = rand() % (int)screen_size.x + 1;
            partile_pos[i].y = 15.f;
            partile_speed[i] = 1 + rand() % 25;
            partile_radius[i] = 10.f;
            partile_size[i] = 3.f;

            partile_target_pos[i].x = rand() % (int)screen_size.x;
            partile_target_pos[i].y = screen_size.y * 2;
        }

        partile_pos[i] = ImLerp(partile_pos[i], partile_target_pos[i], ImGui::GetIO().DeltaTime * (partile_speed[i] / 60));
        partile_rotate[i] += ImGui::GetIO().DeltaTime;

        if (partile_pos[i].y > screen_size.y)
        {
            partile_pos[i].x = 0;
            partile_pos[i].y = 0;
            partile_rotate[i] = 0;
        }

        ImGui::ImRotateStart();
        ImGui::GetWindowDrawList()->AddCircleFilled(partile_pos[i], partile_size[i], ImGui::GetColorU32(color_edit4));
        ImGui::GetWindowDrawList()->AddShadowCircle(partile_pos[i], 1.f, ImGui::GetColorU32(color_edit4), 25.f + partile_size[i], ImVec2(0, 0), 0);
        ImGui::ImRotateEnd(partile_rotate[i]);
    }
}
