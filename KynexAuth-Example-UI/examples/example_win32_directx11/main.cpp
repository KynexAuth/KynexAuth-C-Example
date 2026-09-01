// Dear ImGui: standalone example application for DirectX 11
#include "main.h"
#include "Font.h"
#include "image.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <D3DX11tex.h>
#include <d3d11.h>
#include <tchar.h>
#include <windows.h>
#pragma comment(lib, "D3DX11.lib")
#include <urlmon.h>
#pragma comment(lib, "Urlmon.lib")
#include <atomic>
#include <chrono>
#include <cstdarg>
#include <iostream>
#include <mmsystem.h>
#include <mutex>
#include <thread>
#pragma comment(lib, "Winmm.lib")
#include <Lmcons.h>
#include <ShObjIdl_core.h>
#include <TlHelp32.h>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <dwmapi.h>
#include <filesystem>
#include <future>
#include <iomanip>
#include <random>
#include <shlobj.h>
#include <sstream>
#include <string>
#include <strsafe.h>
#include <vector>
#include <winhttp.h>
#include <wininet.h>

static ID3D11Device*            g_pd3dDevice = NULL;
static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
static IDXGISwapChain*          g_pSwapChain = NULL;
static ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace image {
    ID3D11ShaderResourceView* background = nullptr;
    ID3D11ShaderResourceView* esp_player = nullptr;
    ID3D11ShaderResourceView* logo = nullptr;
    ID3D11ShaderResourceView* Icon_Info = nullptr;
    ID3D11ShaderResourceView* change = nullptr;
    ID3D11ShaderResourceView* exit = nullptr;
    ID3D11ShaderResourceView* icon_combo = nullptr;
}

D3DX11_IMAGE_LOAD_INFO info;
ID3DX11ThreadPump* pump{ nullptr };

inline std::string FormatTimestamp(const std::string& str)
{
    if (str.empty()) return "N/A";
    try {
        time_t t = (time_t)std::stoll(str);
        struct tm tm_info;
        if (localtime_s(&tm_info, &t) == 0) {
            char buf[64];
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_info);
            return std::string(buf);
        }
    } catch (...) {}
    return str;
}

inline std::string tm_to_readable_time(tm ctx)
{
    char buffer[80];
    strftime(buffer, sizeof(buffer), " %m/%d/%Y", &ctx);
    return std::string(buffer);
}

inline static std::time_t string_to_timet(std::string timestamp)
{
    auto cv = strtol(timestamp.c_str(), NULL, 10);
    return (time_t)cv;
}

inline static std::tm timet_to_tm(time_t timestamp)
{
    std::tm context;
    localtime_s(&context, &timestamp);
    return context;
}

inline void Discord_Rpc()
{
    static std::time_t lastUpdateTime = 0;
    std::time_t currentTime = std::time(0);

    if (difftime(currentTime, lastUpdateTime) >= 5)
    {
        std::string detailsText = "❖ F I N E X   C O R P ! ❖";
        std::string current_user = KynexAuthApp.user_data.username.empty() ? (strlen(username) > 0 ? std::string(username) : "") : KynexAuthApp.user_data.username;
        std::string stateText = "» Premium C++ Client • " + current_user;

        DiscordEventHandlers Handle;
        memset(&Handle, 0, sizeof(Handle));
        static bool initialized = false;
        if (!initialized)
        {
            Discord_Initialize("Your Application ID", &Handle, 1, NULL);
            initialized = true;
        }

        DiscordRichPresence discordPresence;
        memset(&discordPresence, 0, sizeof(discordPresence));

        discordPresence.details = detailsText.c_str();
        discordPresence.state = stateText.c_str();
        discordPresence.startTimestamp = currentTime;

        discordPresence.largeImageKey = "https://media2.giphy.com/media/v1.Y2lkPTc5MGI3NjExZWNwZHl6ZmtnMGZoa3B4ZG9xOWszcWloOXl2dzEzczBubGp4azYyOCZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/Z4JFVssA4ZkAIVD8ZK/giphy.gif";
        discordPresence.largeImageText = "FINEX CORP! (C++ Premium)";

        discordPresence.smallImageKey = "https://media1.giphy.com/media/v1.Y2lkPTc5MGI3NjExdDA1NXZkcG80amRkMzlheWFhMDRyN2hnM2NoODhoc2RiaHFvM2hicCZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9cw/wm5tPkFjlRbWu5Oo7w/giphy.gif";
        discordPresence.smallImageText = "</> Dev: FINEX BOYZZ.";

        discordPresence.button1_label = "Join Discord";
        discordPresence.button1_url = "https://discord.gg/AHwg2YA6sE";
        discordPresence.button2_label = "Buy Panel";
        discordPresence.button2_url = "https://wa.link/ywptia";

        Discord_UpdatePresence(&discordPresence);
        Discord_RunCallbacks();

        lastUpdateTime = currentTime;
    }
}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    KynexAuthApp.init();

    WNDCLASSEXW wc;
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = NULL;
    wc.hInstance = nullptr;
    wc.hIcon = nullptr;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = L"F I N E X";
    wc.lpszClassName = L"F I N E X";
    wc.hIconSm = nullptr;

    RegisterClassExW(&wc);
    hwnd = CreateWindowW(wc.lpszClassName, L"F I N E X", WS_POPUP, (GetSystemMetrics(SM_CXSCREEN) / 2) - (HubSize.x / 2), (GetSystemMetrics(SM_CYSCREEN) / 2) - (HubSize.y / 2), HubSize.x, HubSize.y, nullptr, nullptr, wc.hInstance, nullptr);

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    SetWindowLongA(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);

    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    rc = { 0 };
    GetWindowRect(hwnd, &rc);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 0.00f);

    Montserrat   = io.Fonts->AddFontFromMemoryTTF(&Main_Font, sizeof Main_Font, 14.f, NULL, io.Fonts->GetGlyphRangesCyrillic());
    Montserrat_1 = io.Fonts->AddFontFromMemoryTTF(&Main_Font, sizeof Main_Font, 17.f, NULL, io.Fonts->GetGlyphRangesCyrillic());
    Montserrat_2 = io.Fonts->AddFontFromMemoryTTF(&Main_Font, sizeof Main_Font, 13.f, NULL, io.Fonts->GetGlyphRangesCyrillic());

    if (image::change == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, change_byte, sizeof(change_byte), &info, pump, &image::change, 0);
    if (image::exit == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, exit_byte, sizeof(exit_byte), &info, pump, &image::exit, 0);
    if (image::icon_combo == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, icon_combo_byte, sizeof(icon_combo_byte), &info, pump, &image::icon_combo, 0);

    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        Discord_Rpc();
        {
            ImGuiStyle* style = &ImGui::GetStyle();

            if (Hub_Login)
            {
                CustomStyleColor();
                ImGui::SetNextWindowSize(HubSize);
                ImGui::SetNextWindowPos(ImVec2(0, 0));

                ImGui::Begin("Hub_Login", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove);
                {
                    const auto& p = ImGui::GetWindowPos();
                    const ImVec2& region = ImGui::GetContentRegionMax();

                    if (image::background == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, bg_byte, sizeof(bg_byte), &info, pump, &image::background, 0);
                    ImGui::GetWindowDrawList()->AddImageRounded(image::background, ImVec2(p.x, p.y + 50), ImVec2(p.x + region.x, p.y + region.y - 52), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(c::bg_image), 0);

                    if (image::logo == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, logo_byte, sizeof(logo_byte), &info, pump, &image::logo, 0);
                    ImGui::GetWindowDrawList()->AddImageRounded(image::logo, ImVec2(p.x + 308, p.y + 538), ImVec2(p.x + 339, p.y + 569), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(c::text_active), 100);

                    Trinage_background();
                    tab_alpha = ImLerp(tab_alpha, (page == active_tab) ? 1.f : 0.f, 15.f * ImGui::GetIO().DeltaTime);
                    if (tab_alpha < 0.01f && tab_add < 0.01f) active_tab = page;

                    if (iTheme == 0)
                    {
                        ImGui::SetCursorPos(ImVec2(region.x - 53, 11));
                        if (ImGui::Button_Icon("icon", image::change, ImVec2(14, 14))) iTheme = 1;
                    }
                    if (iTheme == 1)
                    {
                        ImGui::SetCursorPos(ImVec2(region.x - 53, 11));
                        if (ImGui::Button_Icon("icon_1", image::change, ImVec2(14, 14))) iTheme = 0;
                    }

                    ImGui::SetCursorPos(ImVec2(region.x - 29, 11));
                    if (ImGui::Button_Icon("exit", image::exit, ImVec2(14, 14))) exit(0);

                    ImGui::GetWindowDrawList()->AddText(Montserrat_1, 17.f, ImVec2(p.x + 120, p.y + 17), ImGui::GetColorU32(c::black), "F I N E X");
                    ImGui::GetWindowDrawList()->AddText(Montserrat_1, 17.f, ImVec2(p.x + 190, p.y + 17), ImGui::GetColorU32(c::black), " C O R P!");

                    login_tab_alpha = ImLerp(login_tab_alpha, (login_tab == active_login_tab) ? 1.f : 0.f, 15.f * ImGui::GetIO().DeltaTime);
                    if (login_tab_alpha < 0.01f) active_login_tab = login_tab;

                    ImGui::SetCursorPos(ImVec2(14, region.y - 38));
                    ImGui::BeginGroup();
                    {
                        if (ImGui::Tab("LOGIN", 0 == login_tab)) login_tab = 0;
                        if (ImGui::Tab("REGISTER", 1 == login_tab)) login_tab = 1;
                        if (ImGui::Tab("LICENSE", 2 == login_tab)) login_tab = 2;
                    }
                    ImGui::EndGroup();

                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, login_tab_alpha * style->Alpha);
                    {
                        if (active_login_tab == 0)
                        {
                            ImGui::SetCursorPos(ImVec2(50, 160));
                            ImGui::BeginGroup();
                            {
                                ImGui::InputTextEx("##username", NULL, "Username", username, IM_ARRAYSIZE(username), ImVec2(250, 36), ImGuiInputTextFlags_None);
                                ImGui::Spacing(); ImGui::Spacing();
                                ImGui::InputTextEx("##password", NULL, "Password", password, IM_ARRAYSIZE(password), ImVec2(250, 36), ImGuiInputTextFlags_Password);
                                ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

                                if (ImGui::Button("Login", ImVec2(250, 36)))
                                {
                                    KynexAuthApp.login(username, password);
                                    if (KynexAuthApp.response.success)
                                    {
                                        Hub_Login = false;
                                        Hub_Home = true;
                                        NotificationManager::Get().Success(KynexAuthApp.response.message, "Welcome");
                                    }
                                    else
                                    {
                                        NotificationManager::Get().Error(KynexAuthApp.response.message, "Login Failed");
                                        memset(password, 0, sizeof(password));
                                    }
                                }
                            }
                            ImGui::EndGroup();
                        }

                        if (active_login_tab == 1)
                        {
                            ImGui::SetCursorPos(ImVec2(50, 130));
                            ImGui::BeginGroup();
                            {
                                ImGui::InputTextEx("##reg_username", NULL, "Username", username, IM_ARRAYSIZE(username), ImVec2(250, 36), ImGuiInputTextFlags_None);
                                ImGui::Spacing(); ImGui::Spacing();
                                ImGui::InputTextEx("##reg_password", NULL, "Password", password, IM_ARRAYSIZE(password), ImVec2(250, 36), ImGuiInputTextFlags_Password);
                                ImGui::Spacing(); ImGui::Spacing();
                                ImGui::InputTextEx("##reg_key", NULL, "License Key", license_key, IM_ARRAYSIZE(license_key), ImVec2(250, 36), ImGuiInputTextFlags_None);
                                ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

                                if (ImGui::Button("Register", ImVec2(250, 36)))
                                {
                                    KynexAuthApp.regstr(username, password, license_key);
                                    if (KynexAuthApp.response.success)
                                    {
                                        Hub_Login = false;
                                        Hub_Home = true;
                                        NotificationManager::Get().Success("Account registered successfully!", "Register Success");
                                    }
                                    else
                                    {
                                        NotificationManager::Get().Error(KynexAuthApp.response.message, "Register Failed");
                                    }
                                }
                            }
                            ImGui::EndGroup();
                        }

                        if (active_login_tab == 2)
                        {
                            ImGui::SetCursorPos(ImVec2(50, 175));
                            ImGui::BeginGroup();
                            {
                                ImGui::InputTextEx("##lic_key", NULL, "License Key", license_key, IM_ARRAYSIZE(license_key), ImVec2(250, 36), ImGuiInputTextFlags_None);
                                ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

                                if (ImGui::Button("License Login", ImVec2(250, 36)))
                                {
                                    KynexAuthApp.license(license_key);
                                    if (KynexAuthApp.response.success)
                                    {
                                        Hub_Login = false;
                                        Hub_Home = true;
                                        NotificationManager::Get().Success("License key validated!", "License Login Success");
                                    }
                                    else
                                    {
                                        NotificationManager::Get().Error(KynexAuthApp.response.message, "License Login Failed");
                                    }
                                }
                            }
                            ImGui::EndGroup();
                        }
                    }
                    ImGui::PopStyleVar();
                    move_window();
                }
                ImGui::End();
            }

            if (Hub_Home)
            {
                CustomStyleColor();
                ImGui::SetNextWindowSize(HubSize);
                ImGui::SetNextWindowPos(ImVec2(0, 0));

                ImGui::Begin("Hub_Home", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove);
                {
                    const auto& p = ImGui::GetWindowPos();
                    const ImVec2& region = ImGui::GetContentRegionMax();

                    if (image::background == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, bg_byte, sizeof(bg_byte), &info, pump, &image::background, 0);
                    ImGui::GetWindowDrawList()->AddImageRounded(image::background, ImVec2(p.x, p.y + 50), ImVec2(p.x + region.x, p.y + region.y - 52), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(c::bg_image), 0);

                    if (image::logo == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, logo_byte, sizeof(logo_byte), &info, pump, &image::logo, 0);
                    ImGui::GetWindowDrawList()->AddImageRounded(image::logo, ImVec2(p.x + 308, p.y + 538), ImVec2(p.x + 339, p.y + 569), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(c::text_active), 100);

                    Trinage_background();
                    tab_alpha = ImLerp(tab_alpha, (page == active_tab) ? 1.f : 0.f, 15.f * ImGui::GetIO().DeltaTime);
                    if (tab_alpha < 0.01f && tab_add < 0.01f) active_tab = page;

                    if (iTheme == 0)
                    {
                        ImGui::SetCursorPos(ImVec2(region.x - 53, 11));
                        if (ImGui::Button_Icon("icon", image::change, ImVec2(14, 14))) iTheme = 1;
                    }
                    if (iTheme == 1)
                    {
                        ImGui::SetCursorPos(ImVec2(region.x - 53, 11));
                        if (ImGui::Button_Icon("icon_1", image::change, ImVec2(14, 14))) iTheme = 0;
                    }

                    ImGui::SetCursorPos(ImVec2(region.x - 29, 11));
                    if (ImGui::Button_Icon("exit", image::exit, ImVec2(14, 14))) exit(0);

                    ImGui::GetWindowDrawList()->AddText(Montserrat_1, 17.f, ImVec2(p.x + 120, p.y + 17), ImGui::GetColorU32(c::black), "F I N E X");
                    ImGui::GetWindowDrawList()->AddText(Montserrat_1, 17.f, ImVec2(p.x + 180, p.y + 17), ImGui::GetColorU32(c::black), " C O R P!");

                    ImGui::SetCursorPos(ImVec2(14, region.y - 38));
                    ImGui::BeginGroup();
                    {
                        if (ImGui::Tab("AIMBOT", 0 == page)) page = 0;
                        if (ImGui::Tab("ESP", 1 == page)) page = 1;
                        if (ImGui::Tab("SAVE", 2 == page)) page = 2;
                        if (ImGui::Tab("PLAYER", 3 == page)) page = 3;
                        if (ImGui::Tab("INFO", 4 == page)) page = 4;
                    }
                    ImGui::EndGroup();

                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, tab_alpha * style->Alpha);
                    {
                        if (active_tab == 0)
                        {
                            ImGui::SetCursorPos(ImVec2(0, 100 - (tab_alpha * 50)));
                            ImGui::BeginChild("beginchild", image::Icon_Info, ImVec2(350, 477), false);
                            {
                                static bool Aimbot = true;
                                ImGui::Checkbox("Aimbot", &Aimbot);
                                ImGui::Separator();

                                static bool Box2D = false;
                                ImGui::Checkbox("Box2D:", &Box2D);
                                ImGui::Separator();

                                static bool Skeleton = true;
                                ImGui::Checkbox("Skeleton", &Skeleton);
                                ImGui::Separator();

                                static bool Lines = true;
                                ImGui::Checkbox("Lines", &Lines);
                                ImGui::Separator();

                                static int Hit = 10;
                                ImGui::SliderInt("Hit Chance", &Hit, 0, 100, "%d%%");
                                ImGui::Separator();

                                ImGui::Combo("Body Aim", image::icon_combo, &select1, items1, IM_ARRAYSIZE(items1), 5, 180);
                                ImGui::Separator();

                                static bool Radar = true;
                                ImGui::Checkbox("Radar", &Radar);
                                ImGui::Separator();

                                static bool BunnyHop = false;
                                ImGui::Checkbox("BunnyHop", &BunnyHop);
                                ImGui::Separator();

                                static int SpeedHack = 50;
                                ImGui::SliderInt("SpeedHack", &SpeedHack, 0, 100, "%d%%");
                                ImGui::Separator();

                                static bool ColorBox = false;
                                ImGui::Checkbox("ColorBox", &ColorBox); ImGui::SameLine(ImGui::GetWindowWidth() - 405); ImGui::ColorEdit4("##color", (float*)&color_edit4, ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip);
                                ImGui::Separator();

                                static bool ColorBox_Esp = true;
                                ImGui::Checkbox("ColorBox Esp", &ColorBox_Esp); ImGui::SameLine(ImGui::GetWindowWidth() - 405); ImGui::ColorEdit4("##color1", (float*)&color_edit4_2, ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip);
                            }
                            ImGui::EndChild();
                        }

                        if (active_tab == 1)
                        {
                            ImGui::SetCursorPos(ImVec2(0, 100 - (tab_alpha * 50)));
                            ImGui::BeginChild("beginchild1", image::Icon_Info, ImVec2(350, 477), false);
                            {
                                static bool ColorBox = false;
                                ImGui::Checkbox("Esp Vehicle", &ColorBox); ImGui::SameLine(ImGui::GetWindowWidth() - 405); ImGui::ColorEdit4("##color2", (float*)&color_edit4_1, ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip);

                                static int Hit = 10;
                                ImGui::SliderInt("Hit Chance", &Hit, 0, 100, "%d%%");
                                ImGui::Separator();

                                ImGui::Combo("Body Aim", image::icon_combo, &select1, items1, IM_ARRAYSIZE(items1), 5, 180);
                                ImGui::Separator();

                                static bool Radar = true;
                                ImGui::Checkbox("Radar", &Radar);
                                ImGui::Separator();

                                static bool BunnyHop = false;
                                ImGui::Checkbox("BunnyHop", &BunnyHop);
                                ImGui::Separator();

                                static int SpeedHack = 50;
                                ImGui::SliderInt("SpeedHack", &SpeedHack, 0, 100, "%d%%");
                                ImGui::Separator();

                                static bool ColorBox1 = false;
                                ImGui::Checkbox("ColorBox", &ColorBox1); ImGui::SameLine(ImGui::GetWindowWidth() - 405); ImGui::ColorEdit4("##color3", (float*)&color_edit4_3, ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip);
                                ImGui::Separator();

                                static bool ColorBox_Esp = true;
                                ImGui::Checkbox("ColorBox Esp", &ColorBox_Esp); ImGui::SameLine(ImGui::GetWindowWidth() - 405); ImGui::ColorEdit4("##color4", (float*)&color_edit4_3, ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip);

                                static bool Aimbot = true;
                                ImGui::Checkbox("Aimbot", &Aimbot);
                                ImGui::Separator();

                                static bool Box2D = false;
                                ImGui::Checkbox("Box2D:", &Box2D);
                                ImGui::Separator();

                                static bool Skeleton = true;
                                ImGui::Checkbox("Skeleton", &Skeleton);
                                ImGui::Separator();

                                static bool Lines = true;
                                ImGui::Checkbox("Lines", &Lines);
                                ImGui::Separator();
                            }
                            ImGui::EndChild();
                        }

                        if (active_tab == 2)
                        {
                            ImGui::SetCursorPos(ImVec2(0, 100 - (tab_alpha * 50)));
                            ImGui::BeginChild("beginchild2", image::Icon_Info, ImVec2(350, 477), false);
                            {
                                static char cfg_name[64] = "Default";
                                ImGui::SetCursorPos(ImVec2(50, 40));
                                ImGui::InputTextEx("##cfg_name", NULL, "Config Name", cfg_name, IM_ARRAYSIZE(cfg_name), ImVec2(250, 36), ImGuiInputTextFlags_None);

                                ImGui::SetCursorPos(ImVec2(50, 95));
                                if (ImGui::Button("Save Config", ImVec2(250, 36)))
                                {
                                    NotificationManager::Get().Success("Config saved as " + std::string(cfg_name), "CONFIG SAVED");
                                }

                                ImGui::SetCursorPos(ImVec2(50, 145));
                                if (ImGui::Button("Load Config", ImVec2(250, 36)))
                                {
                                    NotificationManager::Get().Info("Config " + std::string(cfg_name) + " loaded", "CONFIG LOADED");
                                }

                                ImGui::SetCursorPos(ImVec2(50, 195));
                                if (ImGui::Button("Reset Config", ImVec2(250, 36)))
                                {
                                    NotificationManager::Get().Warning("Config reset to default settings", "CONFIG RESET");
                                }
                            }
                            ImGui::EndChild();
                        }

                        if (active_tab == 3)
                        {
                            if (image::esp_player == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, esp_player_byte, sizeof(esp_player_byte), &info, pump, &image::esp_player, 0);
                            ImGui::GetWindowDrawList()->AddImageRounded(image::esp_player, ImVec2(p.x + 5, p.y + 100), ImVec2(p.x + 340, p.y + 500), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(c::text_active), 0);

                            static bool esp = true;
                            ImGui::SetCursorPos(ImVec2(155, 135));
                            ImGui::Checkbox_Esp("esp", &esp);

                            static bool esp1 = false;
                            ImGui::SetCursorPos(ImVec2(155, 220));
                            ImGui::Checkbox_Esp("esp1", &esp1);

                            static bool esp2 = true;
                            ImGui::SetCursorPos(ImVec2(105, 260));
                            ImGui::Checkbox_Esp("esp2", &esp2);

                            static bool esp3 = true;
                            ImGui::SetCursorPos(ImVec2(215, 260));
                            ImGui::Checkbox_Esp("esp3", &esp3);
                        }

                        if (active_tab == 4)
                        {
                            ImGui::SetCursorPos(ImVec2(0, 100 - (tab_alpha * 50)));
                            ImGui::BeginChild("beginchild4", image::Icon_Info, ImVec2(350, 477), false);
                            {
                                auto DrawInfoRow = [](const char* title, const std::string& value, bool is_accent = false)
                                {
                                    ImVec2 screen_pos = ImGui::GetCursorScreenPos();
                                    float width = 310.0f;
                                    float height = 50.0f;

                                    ImDrawList* draw = ImGui::GetWindowDrawList();
                                    ImVec2 min_p = screen_pos;
                                    ImVec2 max_p = ImVec2(screen_pos.x + width, screen_pos.y + height);

                                    draw->AddRectFilled(min_p, max_p, ImGui::GetColorU32(c::input_bg), 6.0f);
                                    draw->AddRect(min_p, max_p, ImColor(255, 255, 255, 12), 6.0f);

                                    if (is_accent)
                                    {
                                        draw->AddRectFilled(ImVec2(min_p.x + 3.0f, min_p.y + 11.0f), ImVec2(min_p.x + 6.0f, max_p.y - 11.0f), ImGui::GetColorU32(c::main_color), 1.5f);
                                    }

                                    draw->AddText(Montserrat_2, 13.0f, ImVec2(min_p.x + 14.0f, min_p.y + 7.0f), ImGui::GetColorU32(ImVec4(0.60f, 0.60f, 0.65f, 1.0f)), title);

                                    ImU32 val_color = is_accent ? ImGui::GetColorU32(c::main_color) : ImGui::GetColorU32(ImVec4(0.96f, 0.96f, 0.98f, 1.0f));
                                    draw->AddText(Montserrat, 14.0f, ImVec2(min_p.x + 14.0f, min_p.y + 25.0f), val_color, value.c_str());

                                    ImGui::Dummy(ImVec2(width, height + 6.0f));
                                };

                                ImGui::SetCursorPos(ImVec2(20, 20));
                                DrawInfoRow("USERNAME", KynexAuthApp.user_data.username, true);

                                ImGui::SetCursorPos(ImVec2(20, ImGui::GetCursorPosY()));
                                DrawInfoRow("HWID", KynexAuthApp.user_data.hwid);

                                ImGui::SetCursorPos(ImVec2(20, ImGui::GetCursorPosY()));
                                DrawInfoRow("REGISTERED DATE", FormatTimestamp(KynexAuthApp.user_data.createdate));

                                ImGui::SetCursorPos(ImVec2(20, ImGui::GetCursorPosY()));
                                DrawInfoRow("EXPIRY DATE", KynexAuthApp.user_data.subscriptions.empty() ? "" : FormatTimestamp(KynexAuthApp.user_data.subscriptions[0].expiry), true);

                                ImGui::SetCursorPos(ImVec2(20, ImGui::GetCursorPosY()));
                                DrawInfoRow("IP ADDRESS", KynexAuthApp.user_data.ip);
                            }
                            ImGui::EndChild();
                        }
                    }
                    ImGui::PopStyleVar();
                    move_window();
                }
                ImGui::End();
            }
        }

        NotificationManager::Get().Render(ImGui::GetForegroundDrawList(), Montserrat, Montserrat_2, ImVec2(0, 0), HubSize);
        ImGui::Render();

        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0);
    }

    // Cleanup
    Discord_Shutdown();
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_WARP, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
