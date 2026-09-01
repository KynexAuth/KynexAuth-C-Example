#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>
#include "imgui.h"
#include "imgui_internal.h"

enum NotificationType
{
    Notify_Success = 0,
    Notify_Error,
    Notify_Warning,
    Notify_Info
};

struct Notification
{
    std::string title;
    std::string message;
    NotificationType type;
    float duration;
    float time_created;
    float current_alpha;
    float current_y_offset;
};

class NotificationManager
{
private:
    std::vector<Notification> notifications;

public:
    static NotificationManager& Get()
    {
        static NotificationManager instance;
        return instance;
    }

    void Add(const std::string& message, NotificationType type = Notify_Info, const std::string& title = "", float duration = 3.5f)
    {
        std::string actual_title = title;
        if (actual_title.empty())
        {
            switch (type)
            {
            case Notify_Success: actual_title = "SUCCESS"; break;
            case Notify_Error:   actual_title = "ERROR"; break;
            case Notify_Warning: actual_title = "WARNING"; break;
            case Notify_Info:    actual_title = "INFO"; break;
            }
        }

        Notification n;
        n.title = actual_title;
        n.message = message;
        n.type = type;
        n.duration = duration;
        n.time_created = (float)ImGui::GetTime();
        n.current_alpha = 0.0f;
        n.current_y_offset = 30.0f;

        notifications.push_back(n);
    }

    void Success(const std::string& message, const std::string& title = "SUCCESS", float duration = 3.5f)
    {
        Add(message, Notify_Success, title, duration);
    }

    void Error(const std::string& message, const std::string& title = "ERROR", float duration = 3.5f)
    {
        Add(message, Notify_Error, title, duration);
    }

    void Warning(const std::string& message, const std::string& title = "WARNING", float duration = 3.5f)
    {
        Add(message, Notify_Warning, title, duration);
    }

    void Info(const std::string& message, const std::string& title = "INFO", float duration = 3.5f)
    {
        Add(message, Notify_Info, title, duration);
    }

    void Render(ImDrawList* draw_list, ImFont* title_font, ImFont* body_font, const ImVec2& window_pos, const ImVec2& window_size)
    {
        if (notifications.empty())
            return;

        float current_time = (float)ImGui::GetTime();
        float delta_time = ImGui::GetIO().DeltaTime;
        float stack_y = 0.0f;

        for (int i = (int)notifications.size() - 1; i >= 0; --i)
        {
            Notification& notif = notifications[i];
            float elapsed = current_time - notif.time_created;
            float remaining = notif.duration - elapsed;

            if (remaining <= 0.0f)
            {
                notif.current_alpha = ImLerp(notif.current_alpha, 0.0f, delta_time * 10.0f);
                notif.current_y_offset = ImLerp(notif.current_y_offset, 25.0f, delta_time * 10.0f);

                if (notif.current_alpha <= 0.02f)
                {
                    notifications.erase(notifications.begin() + i);
                    continue;
                }
            }
            else
            {
                notif.current_alpha = ImLerp(notif.current_alpha, 1.0f, delta_time * 12.0f);
                notif.current_y_offset = ImLerp(notif.current_y_offset, 0.0f, delta_time * 12.0f);
            }

            float alpha = notif.current_alpha;
            if (alpha > 0.01f)
            {
                const float card_width = 310.0f;
                const float card_height = 52.0f;
                const float card_x = window_pos.x + (window_size.x - card_width) * 0.5f;
                const float card_y = window_pos.y + window_size.y - 75.0f - stack_y + notif.current_y_offset;

                ImVec2 card_min = ImVec2(card_x, card_y);
                ImVec2 card_max = ImVec2(card_x + card_width, card_y + card_height);

                ImVec4 accent_color;
                switch (notif.type)
                {
                case Notify_Success: accent_color = ImVec4(0.18f, 0.88f, 0.55f, alpha); break;
                case Notify_Error:   accent_color = ImVec4(1.00f, 0.28f, 0.35f, alpha); break;
                case Notify_Warning: accent_color = ImVec4(1.00f, 0.65f, 0.15f, alpha); break;
                case Notify_Info:
                default:             accent_color = ImVec4(0.00f, 0.55f, 1.00f, alpha); break;
                }

                ImU32 col_shadow = ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 0.60f * alpha));
                ImU32 col_bg = ImGui::ColorConvertFloat4ToU32(ImVec4(0.06f, 0.06f, 0.09f, 0.96f * alpha));
                ImU32 col_border = ImGui::ColorConvertFloat4ToU32(ImVec4(accent_color.x, accent_color.y, accent_color.z, 0.50f * alpha));
                ImU32 col_accent = ImGui::ColorConvertFloat4ToU32(accent_color);
                ImU32 col_title = ImGui::ColorConvertFloat4ToU32(ImVec4(accent_color.x, accent_color.y, accent_color.z, alpha));
                ImU32 col_text = ImGui::ColorConvertFloat4ToU32(ImVec4(0.92f, 0.92f, 0.95f, 0.95f * alpha));

                // Outer Shadow
                draw_list->AddRectFilled(
                    ImVec2(card_min.x - 2.0f, card_min.y + 2.0f),
                    ImVec2(card_max.x + 2.0f, card_max.y + 4.0f),
                    col_shadow, 8.0f
                );

                // Card Body & Accent Border
                draw_list->AddRectFilled(card_min, card_max, col_bg, 7.0f);
                draw_list->AddRect(card_min, card_max, col_border, 7.0f, 0, 1.2f);

                // Left Accent Glow Pill
                draw_list->AddRectFilled(
                    ImVec2(card_min.x + 5.0f, card_min.y + 9.0f),
                    ImVec2(card_min.x + 9.0f, card_max.y - 9.0f),
                    col_accent, 2.0f
                );

                // Font pointers - ensure native unscaled fonts are used
                ImFont* f_title = title_font ? title_font : GImGui->Font;
                ImFont* f_body = body_font ? body_font : GImGui->Font;

                // Sharp unscaled Title
                draw_list->AddText(f_title, f_title->FontSize, ImVec2(card_min.x + 18.0f, card_min.y + 7.0f), col_title, notif.title.c_str());

                // Sharp unscaled Message
                draw_list->AddText(f_body, f_body->FontSize, ImVec2(card_min.x + 18.0f, card_min.y + 27.0f), col_text, notif.message.c_str(), NULL, card_width - 26.0f);

                // Bottom Countdown Line
                float progress = (std::max)(0.0f, (std::min)(1.0f, remaining / notif.duration));
                float bar_width = (card_width - 16.0f) * progress;
                draw_list->AddRectFilled(
                    ImVec2(card_min.x + 8.0f, card_max.y - 2.5f),
                    ImVec2(card_min.x + 8.0f + bar_width, card_max.y - 1.0f),
                    col_accent, 1.0f
                );

                stack_y += card_height + 8.0f;
            }
        }
    }
};
