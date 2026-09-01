# 🛡️ KynexAuth C++ SDK & Integration Guide

[![C++ Standard](https://img.shields.io/badge/C%2B%2B-17%20%2F%2020-blue.svg)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/Platform-Windows%20(x86%20%2F%20x64)-lightgrey.svg)](https://microsoft.com/windows)
[![GUI Ready](https://img.shields.io/badge/GUI-ImGui%20%7C%20DirectX%20%7C%20Win32-purple.svg)]()
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Status](https://img.shields.io/badge/Status-Stable-brightgreen.svg)]()

Welcome to the official **KynexAuth C++ Client SDK** documentation. This guide walks you through setting up and integrating secure licensing, user authentication, HWID locking, and cloud variables into both **Console Applications** and **GUI Applications (Dear ImGui, DirectX 9/11/12, Win32, Qt)**.

---

## 📺 Video Tutorial

> 🎥 **Step-by-Step Video Walkthrough:**  
> [![Watch Video Tutorial](https://img.shields.io/badge/YouTube-Watch%20Tutorial%20Video-red?style=for-the-badge&logo=youtube)](https://www.youtube.com/watch?v=YOUR_VIDEO_ID_HERE)  
> *(Link: `https://www.youtube.com/watch?v=YOUR_VIDEO_ID_HERE` — Replace with your video URL)*

---

## ✨ Features

- **Robust Authentication**: Login with username/password or direct license key.
- **Hardware ID (HWID) Locking**: Prevents account sharing with automatic machine binding and lock verification.
- **Expiry & Subscription Handling**: Precise timestamp calculations for account and subscription expiration.
- **Cloud Variables**: Securely fetch secret keys, strings, or configs from the server at runtime.
- **Webhooks & Audit Logs**: Trigger Discord/Custom webhooks and transmit client logs directly to the dashboard.
- **Anti-Bypass Protection**: Integrated thread verification and string encryption (`skCrypt`).
- **GUI & Thread Safe**: Works asynchronously with non-blocking threads in ImGui, DirectX, and Win32 applications.
- **Zero-Crash Exception Safety**: Safe JSON deserialization and connection fallback protection.

---

## 📋 Prerequisites

Before getting started, make sure you have:
1. **IDE**: [Visual Studio 2019 or 2022](https://visualstudio.microsoft.com/) with **Desktop development with C++** installed.
2. **C++ Language Standard**: ISO C++17 Standard (`/std:c++17`) or ISO C++20 (`/std:c++20`).
3. **Platform Toolset**: Visual Studio 2019 (v142) or Visual Studio 2022 (v143).
4. **Target OS**: Windows 10 / 11 (x86 or x64).
5. An active application configured on the [KynexAuth Dashboard](https://kynexauth.com).

---

## 🛠️ Visual Studio Project Configuration

### 1. General & Language Configuration
- Open **Project Properties** (`Alt + F7`).
- Navigate to: **Configuration Properties** ➔ **General**.
  - **C++ Language Standard**: Select `ISO C++17 Standard (/std:c++17)` or higher.
- Navigate to: **C/C++** ➔ **Code Generation**.
  - **Runtime Library**: Select `Multi-threaded (/MT)` for Release builds, or `Multi-threaded Debug (/MTd)` for Debug builds.

### 2. Include Directories
- Navigate to: **C/C++** ➔ **General** ➔ **Additional Include Directories**.
- Add the path to the `Authorization` folder containing:
  - `Auth.hpp`
  - `json.hpp` (nlohmann json)
  - `skStr.h` (String encryption)

### 3. Library & Linker Dependencies
- Navigate to: **Linker** ➔ **Input** ➔ **Additional Dependencies**.
- Add the required Windows networking and cryptographic libraries:
  ```text
  ws2_32.lib
  wldap32.lib
  crypt32.lib
  Normaliz.lib
  ```

---

## 💻 Part 1: Console Application Integration

### Step 1: Initialize the Application Object
Instantiate the `api` class with your dashboard credentials. Use `skCrypt` to protect your sensitive keys against reverse-engineering:

```cpp
#include <iostream>
#include <Windows.h>
#include "Authorization/Auth.hpp"
#include "skStr.h"

using namespace KynexAuth;

api KynexAuthApp(
    AppName{ skCrypt("YOUR_APP_NAME").decrypt() },       // App Name from dashboard
    OwnerID{ skCrypt("YOUR_APP_KEY").decrypt() },        // App Key (Public Owner ID)
    Version{ skCrypt("1.0").decrypt() },                 // App Version matching dashboard
    AppUrl{ skCrypt("https://kynexauth.com/api/v1/client").decrypt() } // API Endpoint
);
```

---

### Step 2: Initialize Connection (`init`)
Call `init()` at the entry point of your program (`main()`). This performs version verification and creates a secure session:

```cpp
int main()
{
    std::cout << "\n Connecting to server...";

    KynexAuthApp.init();

    if (!KynexAuthApp.response.success)
    {
        std::cout << "\n Error: " << KynexAuthApp.response.message;
        
        // Optional: Open auto-update link if version mismatch occurs
        if (!KynexAuthApp.app_data.downloadLink.empty()) {
            std::cout << "\n Opening update download link...";
            ShellExecuteA(NULL, "open", KynexAuthApp.app_data.downloadLink.c_str(), NULL, NULL, SW_SHOWNORMAL);
        }

        Sleep(3000);
        return 1;
    }

    std::cout << "\n Connected successfully!";
    // Proceed to Login / Registration
}
```

---

### Step 3: Console Login / Registration Example

```cpp
// 1. Username & Password Login
KynexAuthApp.login("my_username", "my_password");
if (KynexAuthApp.response.success) {
    std::cout << "\n Welcome, " << KynexAuthApp.user_data.username;
    std::cout << "\n Expiry: " << KynexAuthApp.user_data.subscriptions[0].expiry;
}

// 2. Registration with License Key
KynexAuthApp.regstr("my_username", "my_password", "LICENSE-XXXX-XXXX");

// 3. License Only Login
KynexAuthApp.license("LICENSE-XXXX-XXXX");
```

---

## 🎨 Part 2: GUI Application Integration (Dear ImGui, DirectX, Win32)

When integrating into GUI applications (such as **Dear ImGui**, **DirectX 9/11/12 Hook**, **Qt**, or **Win32**), network requests should be executed asynchronously so the **UI frame rate does not freeze / lag** during server communication.

---

### 1. State Management & Asynchronous Threading

Define an `AuthState` enum and atomic flags to manage UI screens and non-blocking background threads:

```cpp
#include "Authorization/Auth.hpp"
#include "skStr.h"
#include <thread>
#include <atomic>
#include <string>

using namespace KynexAuth;

// Global App Instance
api KynexAuthApp(
    AppName{ skCrypt("YOUR_APP_NAME").decrypt() },
    OwnerID{ skCrypt("YOUR_APP_KEY").decrypt() },
    Version{ skCrypt("1.0").decrypt() },
    AppUrl{ skCrypt("https://kynexauth.com/api/v1/client").decrypt() }
);

// GUI States
enum class AuthState {
    Initializing,
    LoginForm,
    RegisterForm,
    LicenseOnlyForm,
    LoggedInDashboard,
    ErrorScreen
};

// UI State Variables
AuthState currentAuthState = AuthState::Initializing;
std::atomic<bool> isBusy(false);
std::string statusMessage = "Connecting to KynexAuth servers...";

// Input Buffers for ImGui
char inputUsername[64] = "";
char inputPassword[64] = "";
char inputLicenseKey[64] = "";
```

---

### 2. Non-Blocking Async Network Functions

Run `init`, `login`, and `register` inside detached `std::thread`s so your rendering loop continues smoothly:

```cpp
// 1. Async Init Call (Call this on application startup)
void StartAsyncInit() {
    isBusy = true;
    currentAuthState = AuthState::Initializing;
    statusMessage = "Initializing security session...";

    std::thread([]() {
        KynexAuthApp.init();
        isBusy = false;

        if (KynexAuthApp.response.success) {
            currentAuthState = AuthState::LoginForm;
            statusMessage = "Ready to authenticate.";
        } else {
            currentAuthState = AuthState::ErrorScreen;
            statusMessage = KynexAuthApp.response.message;
        }
    }).detach();
}

// 2. Async User Login Call
void StartAsyncLogin(const std::string& user, const std::string& pass) {
    if (isBusy) return;
    isBusy = true;
    statusMessage = "Authenticating user...";

    std::thread([user, pass]() {
        KynexAuthApp.login(user, pass);
        isBusy = false;

        if (KynexAuthApp.response.success) {
            currentAuthState = AuthState::LoggedInDashboard;
            statusMessage = "Welcome back, " + KynexAuthApp.user_data.username;
        } else {
            statusMessage = KynexAuthApp.response.message;
        }
    }).detach();
}

// 3. Async License Only Call
void StartAsyncLicenseLogin(const std::string& key) {
    if (isBusy) return;
    isBusy = true;
    statusMessage = "Verifying license...";

    std::thread([key]() {
        KynexAuthApp.license(key);
        isBusy = false;

        if (KynexAuthApp.response.success) {
            currentAuthState = AuthState::LoggedInDashboard;
            statusMessage = "License verified successfully!";
        } else {
            statusMessage = KynexAuthApp.response.message;
        }
    }).detach();
}
```

---

### 3. Rendering the GUI inside Dear ImGui / DirectX

Place this render logic inside your main ImGui frame function (`ImGui::NewFrame()` / `ImGui::Render()`):

```cpp
#include "imgui.h"

void RenderKynexAuthGUI()
{
    ImGui::SetNextWindowSize(ImVec2(420, 320), ImGuiCond_FirstUseEver);
    ImGui::Begin("KynexAuth Security Panel", nullptr, ImGuiWindowFlags_NoCollapse);

    // 1. Initializing / Connecting Screen
    if (currentAuthState == AuthState::Initializing)
    {
        ImGui::Text("Connecting to server, please wait...");
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s", statusMessage.c_str());
    }

    // 2. Login Form Screen
    else if (currentAuthState == AuthState::LoginForm)
    {
        ImGui::Text("Account Login");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::InputText("Username", inputUsername, IM_ARRAYSIZE(inputUsername));
        ImGui::InputText("Password", inputPassword, IM_ARRAYSIZE(inputPassword), ImGuiInputTextFlags_Password);

        ImGui::Spacing();
        if (isBusy) {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Processing...");
        } else {
            if (ImGui::Button("Login", ImVec2(120, 30))) {
                StartAsyncLogin(inputUsername, inputPassword);
            }
            ImGui::SameLine();
            if (ImGui::Button("License Key Login", ImVec2(140, 30))) {
                currentAuthState = AuthState::LicenseOnlyForm;
            }
        }

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", statusMessage.c_str());
    }

    // 3. License Only Login Screen
    else if (currentAuthState == AuthState::LicenseOnlyForm)
    {
        ImGui::Text("Enter License Key");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::InputText("License Key", inputLicenseKey, IM_ARRAYSIZE(inputLicenseKey));

        ImGui::Spacing();
        if (isBusy) {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Verifying key...");
        } else {
            if (ImGui::Button("Activate & Login", ImVec2(140, 30))) {
                StartAsyncLicenseLogin(inputLicenseKey);
            }
            ImGui::SameLine();
            if (ImGui::Button("Back to User Login", ImVec2(140, 30))) {
                currentAuthState = AuthState::LoginForm;
            }
        }

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", statusMessage.c_str());
    }

    // 4. Authenticated Main Dashboard Screen
    else if (currentAuthState == AuthState::LoggedInDashboard)
    {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "Status: Authenticated");
        ImGui::Separator();
        
        ImGui::Text("User: %s", KynexAuthApp.user_data.username.c_str());
        ImGui::Text("HWID: %s", KynexAuthApp.user_data.hwid.c_str());
        ImGui::Text("Created: %s", KynexAuthApp.user_data.createdate.c_str());

        if (!KynexAuthApp.user_data.subscriptions.empty()) {
            ImGui::Text("Subscription: %s", KynexAuthApp.user_data.subscriptions[0].name.c_str());
            ImGui::Text("Expires: %s", KynexAuthApp.user_data.subscriptions[0].expiry.c_str());
        }

        ImGui::Spacing();
        ImGui::Separator();
        
        // Example: Render your Cheat / Application Features here
        ImGui::Text("Welcome to the Software Dashboard!");
        if (ImGui::Button("Launch Feature", ImVec2(140, 30))) {
            KynexAuthApp.log("Feature launched by user");
        }
    }

    // 5. Fatal Error Screen
    else if (currentAuthState == AuthState::ErrorScreen)
    {
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Initialization Failed!");
        ImGui::Separator();
        ImGui::TextWrapped("%s", statusMessage.c_str());
        
        if (ImGui::Button("Retry Connection", ImVec2(140, 30))) {
            StartAsyncInit();
        }
    }

    ImGui::End();
}
```

---

## ⚡ Background Session Heartbeat & Auto-Check

To prevent session timeouts or tampered memory states during long GUI sessions, launch a background session verification thread after successful login:

```cpp
void StartSessionHeartbeat() {
    std::thread([]() {
        while (currentAuthState == AuthState::LoggedInDashboard) {
            // Check session every 60 seconds
            std::this_thread::sleep_for(std::chrono::seconds(60));
            
            KynexAuthApp.check();
            if (!KynexAuthApp.response.success) {
                // Session expired or revoked by admin
                currentAuthState = AuthState::LoginForm;
                statusMessage = "Session expired or terminated by server.";
                break;
            }
        }
    }).detach();
}
```

---

## ❓ Frequently Asked Questions (FAQ) & Troubleshooting

| Issue / Error | Cause | Solution |
| :--- | :--- | :--- |
| **`Version mismatch`** | The `Version` in C++ code does not match the dashboard version. | Update the version in your code or update the version on your KynexAuth dashboard settings. |
| **`Status: Internal Server Error`** | Server connection or session issue. | Ensure the backend server and Redis are running and reachable. |
| **`Linker Error LNK2019 / LNK2001`** | Missing required Windows networking libraries. | Add `ws2_32.lib`, `wldap32.lib`, and `crypt32.lib` to Linker ➔ Input ➔ Additional Dependencies. |
| **GUI freezes during Login** | Network call executed on main render thread. | Use `std::thread` to execute `login()` / `init()` asynchronously as shown above. |
| **Console closes immediately** | Console project finishes execution. | Use `system("pause >nul");` or `std::cin.get();` before exiting `main()`. |

---

## 📄 License & Support

This project is licensed under the **MIT License**.

- 🌐 **Website**: [https://kynexauth.com](https://kynexauth.com)
- 💬 **Discord Support**: [Join Discord](https://discord.gg/kynexauth)
- 📧 **Support Email**: `support@kynexauth.com`
