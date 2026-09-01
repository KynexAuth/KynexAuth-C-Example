#include <Windows.h>
#include "Authorization/Auth.hpp"
#include "auth_guard.hpp"
#include "skStr.h"
#include <chrono>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <limits>
#include <string>
#include <thread>
#undef max

using namespace KynexAuth;

namespace {

void init_fail_delay() {
    Sleep(api::kInitFailSleepMs);
}

void bad_input_delay() {
    Sleep(api::kBadInputSleepMs);
}

void close_delay() {
    Sleep(api::kCloseSleepMs);
}

bool lockout_active(const api::lockout_state& state) {
    return std::chrono::steady_clock::now() < state.locked_until;
}

int lockout_remaining_ms(const api::lockout_state& state) {
    if (!lockout_active(state))
        return 0;

    return static_cast<int>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            state.locked_until - std::chrono::steady_clock::now()).count());
}

void record_login_fail(api::lockout_state& state, int max_attempts = 3, int lock_seconds = 30) {
    if (lockout_active(state))
        return;

    ++state.fails;
    if (state.fails >= max_attempts) {
        state.fails = 0;
        state.locked_until = std::chrono::steady_clock::now() + std::chrono::seconds(lock_seconds);
    }
}

void reset_lockout(api::lockout_state& state) {
    state.fails = 0;
    state.locked_until = std::chrono::steady_clock::time_point{};
}


bool read_int(int& out) {
    std::cin >> out;
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return false; // bad input read. -nigel
    }
    return true;
}

char read_choice(char fallback) {
    char choice = fallback;
    std::cin >> choice;
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        choice = fallback; // default on bad input. -nigel
    }
    return choice;
}



void print_user_data(const api& app) {
    std::cout << "\n User data:";
    std::cout << "\n Username: " << app.user_data.username;
    std::cout << "\n IP address: " << app.user_data.ip;
    std::cout << "\n Hardware-Id: " << app.user_data.hwid;
    std::cout << "\n Create date: " << app.user_data.createdate;
    std::cout << "\n Last login: " << app.user_data.lastlogin;
    std::cout << "\n Subscription(s): ";

    for (size_t i = 0; i < app.user_data.subscriptions.size(); i++) {
        const auto& sub = app.user_data.subscriptions.at(i);
        std::cout << "\n name: " << sub.name;
        std::cout << " : expiry: " << sub.expiry;
        std::cout << " (0)";
    }
}
} // namespace

const std::string compilation_date = (std::string)skCrypt(__DATE__);
const std::string compilation_time = (std::string)skCrypt(__TIME__);
void sessionStatus();


api KynexAuthApp(
    AppName{ skCrypt("FXC#PANEL").decrypt() }, 
    OwnerID{ skCrypt("7lkX4K4g4gqAPDWC9q").decrypt() }, 
    Version{ skCrypt("1.0").decrypt() }, 
    AppUrl{ skCrypt("https://kynexauth.com/api/v1/client").decrypt() }
);
api::lockout_state login_guard{};

int main()
{
    std::string consoleTitle = skCrypt("Loader - Built at:  ").decrypt() + compilation_date + " " + compilation_time;
    SetConsoleTitleA(consoleTitle.c_str());
    std::cout << skCrypt("\n\n Connecting..");

    KynexAuthApp.init();
    if (!KynexAuthApp.response.success)
    {
        std::cout << skCrypt("\n Status: ") << KynexAuthApp.response.message;
        
        if (!KynexAuthApp.app_data.downloadLink.empty()) {
            std::cout << skCrypt("\n\n Opening auto-update link in browser...");
            Sleep(1500);
            ShellExecuteA(NULL, "open", KynexAuthApp.app_data.downloadLink.c_str(), NULL, NULL, SW_SHOWNORMAL);
        }
        
        init_fail_delay();
        exit(1);
    }

    const std::string ownerid_copy = KynexAuthApp.ownerid; // preserve for auth check thread. -nigel

    if (lockout_active(login_guard)) {
        std::cout << skCrypt("\n Status: Too many attempts. Try again in ")
                  << lockout_remaining_ms(login_guard) << skCrypt(" ms.");
        close_delay();
        return 0;
    }

    std::string username;
    std::string password;
    std::string key;
    std::string TfaCode;

    std::cout << skCrypt("\n\n [1] Login\n [2] Register\n [3] Upgrade\n [4] License key only\n\n Choose option: ");

        int option = 0;
        if (!read_int(option))
        {
            std::cout << skCrypt("\n\n Status: Failure: Invalid Selection");
            bad_input_delay();
            exit(1);
        }

        switch (option)
        {
        case 1:
            std::cout << skCrypt("\n\n Enter username: ");
            std::cin >> username;
            std::cout << skCrypt("\n Enter password: ");
            std::cin >> password;
            KynexAuthApp.login(username, password, "");
            break;
        case 2:
            std::cout << skCrypt("\n\n Enter username: ");
            std::cin >> username;
            std::cout << skCrypt("\n Enter password: ");
            std::cin >> password;
            std::cout << skCrypt("\n Enter license: ");
            std::cin >> key;
            KynexAuthApp.regstr(username, password, key);
            break;
        case 3:
            std::cout << skCrypt("\n\n Enter username: ");
            std::cin >> username;
            std::cout << skCrypt("\n Enter license: ");
            std::cin >> key;
            KynexAuthApp.upgrade(username, key);
            break;
        case 4:
            std::cout << skCrypt("\n Enter license: ");
            std::cin >> key;
            KynexAuthApp.license(key, "");
            break;
        default:
            std::cout << skCrypt("\n\n Status: Failure: Invalid Selection");
            bad_input_delay();
            exit(1);
        }

    if (KynexAuthApp.response.message.empty())
        exit(11);

    if (!KynexAuthApp.response.success)
    {
        if (KynexAuthApp.response.message == "2FA code required.") {
            if (username.empty() || password.empty()) {
                std::cout << skCrypt("\n Your account has 2FA enabled, please enter 6-digit code:");
                std::cin >> TfaCode;
                KynexAuthApp.license(key, TfaCode);
            }
            else {
                std::cout << skCrypt("\n Your account has 2FA enabled, please enter 6-digit code:");
                std::cin >> TfaCode;
                KynexAuthApp.login(username, password, TfaCode);
            }

            if (KynexAuthApp.response.message.empty())
                exit(11);
            if (!KynexAuthApp.response.success) {
                std::cout << skCrypt("\n Status: ") << KynexAuthApp.response.message;
                record_login_fail(login_guard);
                init_fail_delay();
                exit(1);
            }
        }
        else {
            std::cout << skCrypt("\n Status: ") << KynexAuthApp.response.message;
            record_login_fail(login_guard);
            init_fail_delay();
            exit(1);
        }
    }
    reset_lockout(login_guard);

    /*
    * Do NOT remove this checkAuthenticated() function.
    * It protects you from cracking, it would be NOT be a good idea to remove it
    */
    std::thread run(checkAuthenticated, ownerid_copy);
    // do NOT remove checkAuthenticated(), it MUST stay for security reasons
    std::thread check(sessionStatus); // do NOT remove this function either.
    run.detach(); // detach immediately to avoid terminate on early exits. -nigel
    check.detach(); // detach immediately to avoid terminate on early exits. -nigel

    //enable 2FA 
    // KynexAuthApp.enable2fa(); you will need to ask for the code
    //enable 2fa without the need of asking for the code
    //KynexAuthApp.enable2fa().handleInput(KynexAuthApp);

    //disbale 2FA
    // KynexAuthApp.disable2fa();

    if (KynexAuthApp.user_data.username.empty())
        exit(10);

    print_user_data(KynexAuthApp);

    std::cout << skCrypt("\n\n Status: ") << KynexAuthApp.response.message;
    std::cout << skCrypt("\n\n Press any key to exit...");
    system("pause >nul");

    return 0;
}

void sessionStatus() {
    KynexAuthApp.check(true); // do NOT specify true usually, it is slower and will get you blocked from API
    if (!KynexAuthApp.response.success) {
        return; // allow clean exit from thread. -nigel
    }

    if (KynexAuthApp.response.isPaid) {
        while (true) {
            Sleep(20000); // this MUST be included or else you get blocked from API
            KynexAuthApp.check();
            if (!KynexAuthApp.response.success) {
                return; // allow clean exit from thread. -nigel
            }
        }
    }
}
