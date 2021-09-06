#include <pch.h>

#include "Gui/Gui.h"

void sequencer() {
	ImGui::Text("This is a sequencer");
}





//#define Sequentity
#ifdef Sequentity

void EventEditor(entt::registry& registry) {
    auto& state = registry.ctx_or_set<State>();

    _transition(state.pan[0], state.target_pan[0], CommonTheme.transition_speed, 1.0f);
    _transition(state.pan[1], state.target_pan[1], CommonTheme.transition_speed, 1.0f);
    _transition(state.zoom[0], state.target_zoom[0], CommonTheme.transition_speed);
    _transition(state.zoom[1], state.target_zoom[1], CommonTheme.transition_speed);

    auto* painter = ImGui::GetWindowDrawList();
    auto titlebarHeight = 24.0f; // TODO: Find an exact value for this
    const ImVec2 windowSize = ImGui::GetWindowSize();
    const ImVec2 windowPos = ImGui::GetWindowPos() + ImVec2{ 0.0f, titlebarHeight };
    const ImVec2 padding{ 7.0f, 2.0f };

    /**
     * @brief Sequentity divided into 4 panels
     *
     *          _________________________________________________
     *         |       |                                         |
     *   Cross |   X   |                  B                      | Timeline
     *         |_______|_________________________________________|
     *         |       |                                         |
     *         |       |                                         |
     *         |       |                                         |
     *  Lister |   A   |                  C                      | Editor
     *         |       |                                         |
     *         |       |                                         |
     *         |_______|_________________________________________|
     *
     */

    auto X = windowPos;
    auto A = windowPos + ImVec2{ 0.0f, TimelineTheme.height };
    auto B = windowPos + ImVec2{ ListerTheme.width, 0.0f };
    auto C = windowPos + ImVec2{ ListerTheme.width, TimelineTheme.height };

    float zoom_ = state.zoom[0] / state.stride;
    int stride_ = state.stride * 5;  // How many frames to skip drawing
    int minTime = state.range[0] / stride_;
    int maxTime = state.range[1] / stride_;

    // TODO: What is this value?
    auto multiplier = zoom_ / stride_;

    auto time_to_px = [&multiplier](TimeType time)  -> float { return time * multiplier; };
    auto px_to_time = [&multiplier](float px)  -> int { return static_cast<int>(px / multiplier); };

    auto CrossBackground = [&]() {
        painter->AddRectFilled(
            X,
            X + ImVec2{ ListerTheme.width + 1, TimelineTheme.height },
            ImColor(ListerTheme.background)
        );

        // Border
        painter->AddLine(X + ImVec2{ ListerTheme.width, 0.0f },
            X + ImVec2{ ListerTheme.width, TimelineTheme.height },
            ImColor(CommonTheme.dark),
            CommonTheme.border_width);

        painter->AddLine(X + ImVec2{ 0.0f, TimelineTheme.height },
            X + ImVec2{ ListerTheme.width + 1, TimelineTheme.height },
            ImColor(CommonTheme.dark),
            CommonTheme.border_width);
    };

    auto ListerBackground = [&]() {
        if (CommonTheme.bling) {
            // Drop Shadow
            painter->AddRectFilled(
                A,
                A + ImVec2{ ListerTheme.width + 3.0f, windowSize.y },
                ImColor(0.0f, 0.0f, 0.0f, 0.1f)
            );
            painter->AddRectFilled(
                A,
                A + ImVec2{ ListerTheme.width + 2.0f, windowSize.y },
                ImColor(0.0f, 0.0f, 0.0f, 0.2f)
            );
        }

        // Fill
        painter->AddRectFilled(
            A,
            A + ImVec2{ ListerTheme.width, windowSize.y },
            ImColor(ImGui::GetStyleColorVec4(ImGuiCol_TitleBg))
        );

        // Border
        painter->AddLine(A + ImVec2{ ListerTheme.width, 0.0f },
            A + ImVec2{ ListerTheme.width, windowSize.y },
            ImColor(CommonTheme.dark),
            CommonTheme.border_width);
    };

    auto TimelineBackground = [&]() {
        if (CommonTheme.bling) {
            // Drop Shadow
            painter->AddRectFilled(
                B,
                B + ImVec2{ windowSize.x, TimelineTheme.height + 3.0f },
                ImColor(0.0f, 0.0f, 0.0f, 0.1f)
            );
            painter->AddRectFilled(
                B,
                B + ImVec2{ windowSize.x, TimelineTheme.height + 2.0f },
                ImColor(0.0f, 0.0f, 0.0f, 0.2f)
            );
        }

        // Fill
        painter->AddRectFilled(
            B,
            B + ImVec2{ windowSize.x, TimelineTheme.height },
            ImColor(TimelineTheme.background)
        );

        // Border
        painter->AddLine(B + ImVec2{ 0.0f, TimelineTheme.height },
            B + ImVec2{ windowSize.x, TimelineTheme.height },
            ImColor(CommonTheme.dark),
            CommonTheme.border_width);
    };

    auto EditorBackground = [&]() {
        painter->AddRectFilled(
            C,
            C + windowPos + windowSize,
            ImColor(EditorTheme.background)
        );
    };

    auto Timeline = [&]() {
        for (TimeType time = minTime; time < maxTime + 1; time++) {
            float xMin = time * zoom_;
            float xMax = 0.0f;
            float yMin = 0.0f;
            float yMax = TimelineTheme.height - 1;

            xMin += B.x + state.pan[0];
            xMax += B.x + state.pan[0];
            yMin += B.y;
            yMax += B.y;

            painter->AddLine(ImVec2(xMin, yMin), ImVec2(xMin, yMax), ImColor(TimelineTheme.dark));
            painter->AddText(
                ImGui::GetFont(),
                ImGui::GetFontSize() * 0.85f,
                ImVec2{ xMin + 5.0f, yMin },
                ImColor(TimelineTheme.text),
                std::to_string(time * stride_).c_str()
            );

            if (time == maxTime) break;

            for (int z = 0; z < 5 - 1; z++) {
                const auto innerSpacing = zoom_ / 5;
                auto subline = innerSpacing * (z + 1);
                painter->AddLine(
                    ImVec2{ xMin + subline, yMin + (TimelineTheme.height * 0.5f) },
                    ImVec2{ xMin + subline, yMax },
                    ImColor(TimelineTheme.mid));
            }
        }
    };
    /**
     * @brief Vertical grid lines
     *
     *   ______________________________________________________
     *  |    |    |    |    |    |    |    |    |    |    |    |
     *  |    |    |    |    |    |    |    |    |    |    |    |
     *  |    |    |    |    |    |    |    |    |    |    |    |
     *  |    |    |    |    |    |    |    |    |    |    |    |
     *  |    |    |    |    |    |    |    |    |    |    |    |
     *  |____|____|____|____|____|____|____|____|____|____|____|
     *
     */
    auto VerticalGrid = [&]() {
        for (TimeType time = minTime; time < maxTime + 1; time++) {
            float xMin = time * zoom_;
            float xMax = 0.0f;
            float yMin = 0.0f;
            float yMax = windowSize.y;

            xMin += C.x + state.pan[0];
            xMax += C.x + state.pan[0];
            yMin += C.y;
            yMax += C.y;

            painter->AddLine(ImVec2(xMin, yMin), ImVec2(xMin, yMax), ImColor(EditorTheme.dark));

            if (time == maxTime) break;

            for (int z = 0; z < 5 - 1; z++) {
                const auto innerSpacing = zoom_ / 5;
                auto subline = innerSpacing * (z + 1);
                painter->AddLine(ImVec2(xMin + subline, yMin), ImVec2(xMin + subline, yMax), ImColor(EditorTheme.mid));
            }
        }
    };

    /**
     * @brief Draw time indicator
     *         __
     *        |  |
     *  _______\/___________________________________________________
     *         |
     *         |
     *         |
     *         |
     *         |
     *
     */
    unsigned int indicator_count{ 0 };
    auto TimeIndicator = [&](TimeType& time, ImVec4 cursor_color, ImVec4 line_color) {
        auto xMin = time * zoom_ / stride_;
        auto xMax = 0.0f;
        auto yMin = TimelineTheme.height;
        auto yMax = windowSize.y;

        xMin += B.x + state.pan[0];
        xMax += B.x + state.pan[0];
        yMin += B.y;
        yMax += B.y;

        painter->AddLine(ImVec2{ xMin, yMin }, ImVec2{ xMin, yMax }, ImColor(line_color), 2.0f);

        /**
         *  Cursor
         *    __
         *   |  |
         *    \/
         *
         */
        ImVec2 size{ 10.0f, 20.0f };
        auto topPos = ImVec2{ xMin, yMin };

        ImGui::PushID(indicator_count);
        ImGui::SetCursorPos(topPos - ImVec2{ size.x, size.y } - ImGui::GetWindowPos());
        ImGui::SetItemAllowOverlap(); // Prioritise the last drawn (shouldn't this be the default?)
        ImGui::InvisibleButton("##indicator", size * 2.0f);
        ImGui::PopID();

        static TimeType initial_time{ 0 };
        if (ImGui::IsItemActivated()) {
            initial_time = time;
        }

        ImVec4 color = cursor_color;
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            color = color * 1.2f;
        }

        if (ImGui::IsItemActive()) {
            time = initial_time + static_cast<int>(ImGui::GetMouseDragDelta().x / (zoom_ / stride_));
        }

        ImVec2 points[5] = {
            topPos,
            topPos - ImVec2{ -size.x, size.y / 2.0f },
            topPos - ImVec2{ -size.x, size.y },
            topPos - ImVec2{ size.x, size.y },
            topPos - ImVec2{ size.x, size.y / 2.0f }
        };

        ImVec2 shadow1[5];
        ImVec2 shadow2[5];
        for (int i = 0; i < 5; i++) { shadow1[i] = points[i] + ImVec2{ 1.0f, 1.0f }; }
        for (int i = 0; i < 5; i++) { shadow2[i] = points[i] + ImVec2{ 3.0f, 3.0f }; }

        painter->AddConvexPolyFilled(shadow1, 5, ImColor(CommonTheme.shadow));
        painter->AddConvexPolyFilled(shadow2, 5, ImColor(CommonTheme.shadow));
        painter->AddConvexPolyFilled(points, 5, ImColor(color));
        painter->AddPolyline(points, 5, ImColor(color * 1.25f), true, 1.0f);
        painter->AddLine(topPos - ImVec2{ 2.0f, size.y * 0.3f },
            topPos - ImVec2{ 2.0f, size.y * 0.8f },
            ImColor(EditorTheme.accent_dark));
        painter->AddLine(topPos - ImVec2{ -2.0f, size.y * 0.3f },
            topPos - ImVec2{ -2.0f, size.y * 0.8f },
            ImColor(EditorTheme.accent_dark));

        indicator_count++;
    };

    /**
     * @brief MIDI-like representation of on/off events
     *
     *   _______       _______________
     *  |_______|     |_______________|
     *       _______________
     *      |_______________|
     *                          _________________________
     *                         |_________________________|
     *
     */
    auto Header = [&](const Track& track, ImVec2& cursor) {
        // Draw track header, a separator-like empty space
        // 
        // |__|__|__|__|__|__|__|__|__|__|__|__|__|__|
        // |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
        // |__|__|__|__|__|__|__|__|__|__|__|__|__|__|
        // |                                         |
        // |_________________________________________|
        // |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
        // |__|__|__|__|__|__|__|__|__|__|__|__|__|__|
        // |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
        //
        const ImVec2 size{ windowSize.x, CommonTheme.track_height };

        // Hide underlying vertical lines
        painter->AddRectFilled(
            ImVec2{ C.x, cursor.y },
            ImVec2{ C.x, cursor.y } + size,
            ImColor(EditorTheme.background)
        );

        if (CommonTheme.tint_track) {
            // Tint empty area with the track color
            painter->AddRectFilled(
                ImVec2{ C.x, cursor.y },
                ImVec2{ C.x, cursor.y } + size,
                ImColor(track.color.x, track.color.y, track.color.z, 0.1f)
            );
        }

        painter->AddRect(
            ImVec2{ C.x, cursor.y },
            ImVec2{ C.x, cursor.y } + size,
            ImColor(EditorTheme.mid)
        );

        cursor.y += size.y;
    };

    auto Events = [&]() {
        ImVec2 cursor{ C.x + state.pan[0], C.y + state.pan[1] };

        registry.view<Track>().each([&](auto& track) {
            Header(track, cursor);

            // Give each event a unique ImGui ID
            unsigned int event_count{ 0 };

            // Draw events
            //            ________
            //           |________|
            //  _________________________
            // |_________________________|
            //     ________      ____________________________
            //    |________|    |____________________________|
            //
            for (auto& [type, channel] : track.channels) {
                for (auto& event : channel.events) {

                    ImVec2 pos{ time_to_px(event.time), 0.0f };
                    ImVec2 size{ time_to_px(event.length), state.zoom[1] };

                    ImVec2 head_tail_size
                    {
                        std::min(EditorTheme.head_tail_handle_width.y,
                            std::max(EditorTheme.head_tail_handle_width.x, size.x / 6)
                        ),
                        size.y
                    };
                    ImVec2 tail_pos{ pos.x + size.x - head_tail_size.x , pos.y };

                    ImVec2 body_pos{ pos.x + head_tail_size.x, pos.y };
                    ImVec2 body_size{ size.x - 2 * head_tail_size.x, size.y };

                    // Transitions
                    float target_height{ 0.0f };
                    float target_thickness = 0.0f;

                    /** Implement crop interaction and visualisation
                         ____________________________________
                        |      |                      |      |
                        | head |         body         | tail |
                        |______|______________________|______|

                    */
                    static int initial_time{ 0 };
                    static int initial_length{ 0 };

                    ImGui::PushID(track.label);
                    ImGui::PushID(event_count);

#pragma region EventHead
                    ImGui::SetCursorPos(cursor + pos - ImGui::GetWindowPos());
                    ImGui::SetItemAllowOverlap();
                    ImGui::InvisibleButton("##event_head", head_tail_size);

                    bool head_hovered = ImGui::IsItemHovered() || ImGui::IsItemActive();

                    if (ImGui::IsItemActivated()) {
                        initial_time = event.time;
                        initial_length = event.length;
                        state.selected_event = &event;
                    }

                    if (!ImGui::GetIO().KeyAlt && ImGui::IsItemActive()) {
                        float delta = ImGui::GetMouseDragDelta().x;
                        event.time = initial_time + px_to_time(delta);
                        event.length = initial_length - px_to_time(delta);
                        event.removed = (
                            event.time > state.range[1] ||
                            event.time + event.length < state.range[0]
                            );

                        event.enabled = !event.removed;
                        target_height = EditorTheme.active_clip_raise / 2;
                    }
#pragma endregion EventHead

#pragma region EventTail
                    ImGui::SetCursorPos(cursor + tail_pos - ImGui::GetWindowPos());
                    ImGui::SetItemAllowOverlap();
                    ImGui::InvisibleButton("##event_tail", head_tail_size);

                    bool tail_hovered = ImGui::IsItemHovered() || ImGui::IsItemActive();

                    if (ImGui::IsItemActivated()) {
                        initial_time = event.time;
                        initial_length = event.length;
                        state.selected_event = &event;
                    }

                    if (!ImGui::GetIO().KeyAlt && ImGui::IsItemActive()) {
                        float delta = ImGui::GetMouseDragDelta().x;
                        event.length = initial_length + px_to_time(delta);
                        event.removed = (
                            event.time > state.range[1] ||
                            event.time + event.length < state.range[0]
                            );

                        event.enabled = !event.removed;
                        target_height = EditorTheme.active_clip_raise / 2;
                    }
#pragma endregion EventTail

#pragma region EventBody
                    ImGui::SetCursorPos(cursor + body_pos - ImGui::GetWindowPos());
                    ImGui::SetItemAllowOverlap();
                    ImGui::InvisibleButton("##event", body_size);

                    ImGui::PopID();
                    ImGui::PopID();

                    ImVec4 color = channel.color;

                    if (!event.enabled || track.mute || track._notsoloed) {
                        color = ImColor::HSV(0.0f, 0.0f, 0.5f);
                    }

                    else if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
                        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                        target_thickness = 2.0f;
                    }

                    // User Input
                    if (ImGui::IsItemActivated()) {
                        initial_time = event.time;
                        initial_length = event.length;
                        state.selected_event = &event;
                    }

                    if (!ImGui::GetIO().KeyAlt && ImGui::IsItemActive()) {
                        float delta = ImGui::GetMouseDragDelta().x;
                        event.time = initial_time + px_to_time(delta);
                        event.removed = (
                            event.time > state.range[1] ||
                            event.time + event.length < state.range[0]
                            );

                        event.enabled = !event.removed;
                        target_height = EditorTheme.active_clip_raise;
                    }
#pragma endregion EventBody

                    _transition(event.height, target_height, CommonTheme.transition_speed);
                    pos -= event.height;
                    tail_pos -= event.height;

                    const int shadow = 2;
                    float shadow_movement = event.height * (1.0f + EditorTheme.active_clip_raise_shadow_movement);
                    painter->AddRectFilled(
                        cursor + pos + shadow + shadow_movement,
                        cursor + pos + size + shadow + shadow_movement,
                        ImColor::HSV(0.0f, 0.0f, 0.0f, 0.3f), EditorTheme.radius
                    );

                    painter->AddRectFilled(
                        cursor + pos,
                        cursor + pos + size,
                        ImColor(color), EditorTheme.radius
                    );

                    // Add a dash to the bottom of each event.
                    painter->AddRectFilled(
                        cursor + pos + ImVec2{ 0.0f, size.y - 5.0f },
                        cursor + pos + size,
                        ImColor(color * 0.8f), EditorTheme.radius
                    );

                    if (ImGui::IsItemHovered() || ImGui::IsItemActive() || head_hovered || tail_hovered || state.selected_event == &event) {
                        painter->AddRect(
                            cursor + pos + event.thickness * 0.25f,
                            cursor + pos + size - event.thickness * 0.25f,
                            ImColor(EditorTheme.selection), EditorTheme.radius, ImDrawCornerFlags_All, event.thickness
                        );
                    }
                    else {
                        painter->AddRect(
                            cursor + pos + event.thickness,
                            cursor + pos + size - event.thickness,
                            ImColor(EditorTheme.outline), EditorTheme.radius
                        );
                    }

                    if (head_hovered)
                    {
                        painter->AddRectFilled(
                            cursor + pos,
                            cursor + pos + head_tail_size,
                            ImColor(EditorTheme.head_tail_hover), EditorTheme.radius
                        );
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                    }

                    if (tail_hovered)
                    {
                        painter->AddRectFilled(
                            cursor + tail_pos,
                            cursor + tail_pos + head_tail_size,
                            ImColor(EditorTheme.head_tail_hover), EditorTheme.radius
                        );
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                    }

                    if (event.enabled) {
                        if (ImGui::IsItemHovered() || ImGui::IsItemActive() || head_hovered || tail_hovered) {
                            if (event.length > 5.0f) {
                                painter->AddText(
                                    ImGui::GetFont(),
                                    ImGui::GetFontSize() * 0.85f,
                                    cursor + pos + ImVec2{ 3.0f + event.thickness, 0.0f },
                                    ImColor(EditorTheme.text),
                                    std::to_string(event.time).c_str()
                                );
                            }

                            if (event.length > 30.0f) {
                                painter->AddText(
                                    ImGui::GetFont(),
                                    ImGui::GetFontSize() * 0.85f,
                                    cursor + pos + ImVec2{ size.x - 20.0f, 0.0f },
                                    ImColor(EditorTheme.text),
                                    std::to_string(event.length).c_str()
                                );
                            }
                        }
                    }

                    event_count++;
                }

                // Next event type
                cursor.y += state.zoom[1] + EditorTheme.spacing;
            }

            // Next track
            cursor.y += padding.y;
            });
    };

    auto Range = [&]() {
        ImVec2 cursor{ C.x, C.y };
        ImVec2 range_cursor_start{ state.range[0] * zoom_ / stride_ + state.pan[0], TimelineTheme.height };
        ImVec2 range_cursor_end{ state.range[1] * zoom_ / stride_ + state.pan[0], TimelineTheme.height };

        painter->AddRectFilled(
            cursor + ImVec2{ 0.0f, 0.0f },
            cursor + ImVec2{ range_cursor_start.x, windowSize.y },
            ImColor(0.0f, 0.0f, 0.0f, 0.3f)
        );

        painter->AddRectFilled(
            cursor + ImVec2{ range_cursor_end.x, 0.0f },
            cursor + ImVec2{ windowSize.x, windowSize.y },
            ImColor(0.0f, 0.0f, 0.0f, 0.3f)
        );
    };

    auto Button = [](const char* label, bool& checked, ImVec2 size = { 20.0f, 20.0f }) -> bool {
        if (checked) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0.25f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0.15f));
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0.1f));
        }

        const bool pressed = ImGui::Button(label, size);

        if (checked) ImGui::PopStyleColor(2);
        else         ImGui::PopStyleColor(1);

        if (pressed) checked ^= true;

        return pressed;
    };

    /**
     * @brief Outliner-style listing of available channels
     *
     *  ______________________________
     * |  _  _  _                     |
     * | |_||_||_|       track A      |
     * |                  |-o chan 1  |
     * |  _  _  _         |-o chan 2  |
     * | |_||_||_|       track B      |
     * |  _  _  _         |-o chan 1  |
     * | |_||_||_|       track C      |
     * |                  |-o chan 1  |
     * |                  |-o chan 2  |
     * |  _  _  _         |-o chan 3  |
     * | |_||_||_|       track D      |
     * |                 ...          |
     * |                              |
     * |______________________________|
     *
     *
     */
    auto Lister = [&]() {
        auto cursor = ImVec2{ A.x, A.y + state.pan[1] };

        registry.view<Track>().each([&](auto& track) {

            // Draw track header
            //  _________________________________________   _
            // |  ______ ______  |                     | |   |
            // | | Mute | Solo | |        Track        | |   | track_height
            // |_________________|_____________________|_|  _|
            //
            // |_________________________________________|
            //                             width
            // |_________________|
            //    buttons_width

            const auto textSize = ImGui::CalcTextSize(track.label);
            const auto pos = ImVec2{
                ListerTheme.width - textSize.x - padding.x - padding.x,
                CommonTheme.track_height / 2.0f - textSize.y / 2.0f
            };

            painter->AddRectFilled(
                cursor + ImVec2{ ListerTheme.width - 5.0f, 0.0f },
                cursor + ImVec2{ ListerTheme.width, CommonTheme.track_height },
                ImColor(track.color)
            );

            painter->AddText(
                cursor + pos, ImColor(ListerTheme.text), track.label
            );

            ImGui::SetCursorPos(cursor + ImVec2{ padding.x, 0.0f } - ImGui::GetWindowPos());
            ImGui::PushID(track.label);
            Button("m", track.mute, { CommonTheme.track_height, CommonTheme.track_height });
            ImGui::SameLine();

            if (Button("s", track.solo, { CommonTheme.track_height, CommonTheme.track_height })) {
                _solo(registry, track);
            }

            ImGui::PopID();

            const auto track_corner = cursor;
            cursor.y += CommonTheme.track_height;

            // Draw channels
            // |
            // |-- channel
            // |-- channel
            // |__ channel
            //
            for (auto& [type, channel] : track.channels) {
                static const ImVec2 indicator_size{ 9.0f, 9.0f };
                const ImVec2 indicator_pos = {
                    ListerTheme.width - indicator_size.x - padding.x,
                    state.zoom[1] * 0.5f - indicator_size.y * 0.5f
                };

                painter->AddRectFilled(
                    cursor + indicator_pos,
                    cursor + indicator_pos + indicator_size,
                    ImColor(channel.color)
                );

                painter->AddRect(
                    cursor + indicator_pos,
                    cursor + indicator_pos + indicator_size,
                    ImColor(channel.color * 1.25f)
                );

                const auto textSize = ImGui::CalcTextSize(channel.label) * 0.85f;
                const auto pos = ImVec2{
                    ListerTheme.width - textSize.x - padding.x - indicator_size.x - padding.x,
                    state.zoom[1] * 0.5f - textSize.y * 0.5f
                };

                painter->AddText(
                    ImGui::GetFont(),
                    textSize.y,
                    cursor + pos,
                    ImColor(ListerTheme.text), channel.label
                );

                // Next channel
                cursor.y += state.zoom[1] + EditorTheme.spacing;
            }

            // Next track
            cursor.y += padding.y;

            // Visualise mute and solo state
            if (track.mute || track._notsoloed) {
                ImVec4 faded = ListerTheme.background;
                faded.w = 0.8f;

                painter->AddRectFilled(
                    track_corner + ImVec2{ ListerTheme.buttons_width, 0.0f },
                    track_corner + ImVec2{ ListerTheme.width, cursor.y },
                    ImColor(faded)
                );
            }
            });
    };

    /**
     * Draw
     *
     */
    EditorBackground();
    VerticalGrid();
    Events();
    TimelineBackground();
    Timeline();
    Range();

    // Can intercept mouse events
    bool hovering_background{ true };
    TimeIndicator(state.range[0], TimelineTheme.start_time, EditorTheme.start_time);
    if (ImGui::IsItemHovered()) hovering_background = false;
    TimeIndicator(state.range[1], TimelineTheme.end_time, EditorTheme.end_time);
    if (ImGui::IsItemHovered()) hovering_background = false;
    TimeIndicator(state.current_time, TimelineTheme.current_time, EditorTheme.current_time);
    if (ImGui::IsItemHovered()) hovering_background = false;

    ListerBackground();
    Lister();
    CrossBackground();

    /**
     * User Input
     *
     */
    if (hovering_background) {

        /** Pan
         *
         *        ^
         *        |
         *        |
         *  <-----|----->
         *        |
         *        |
         *        v
         */
        ImGui::SetCursorPos({ 0.0f, titlebarHeight });
        ImGui::InvisibleButton("##mpan", ImVec2{ ListerTheme.width, TimelineTheme.height });

        if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        const bool panM = (
            ImGui::IsItemActive() ||
            (ImGui::IsWindowFocused() && ImGui::GetIO().KeyAlt && ImGui::GetIO().MouseDown[0])
            );

        /** Vertical Pan
         *
         *
         *
         *  <--------------->
         *
         *
         */
        ImGui::SetCursorPos({ ListerTheme.width, titlebarHeight });
        ImGui::InvisibleButton("##pan[0]", ImVec2{ windowSize.x, TimelineTheme.height });

        if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        const bool panH = ImGui::IsItemActive();

        /** Vertical Pan
         *
         *        ^
         *        |
         *        |
         *        |
         *        v
         */
        ImGui::SetCursorPos({ ListerTheme.width - 110.0f, TimelineTheme.height + titlebarHeight });
        ImGui::InvisibleButton("##pan[1]", ImVec2{ ListerTheme.width, windowSize.y });

        if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
        const bool panV = ImGui::IsItemActive();

        if (panM) {
            state.target_pan[0] += ImGui::GetIO().MouseDelta.x;
            state.target_pan[1] += ImGui::GetIO().MouseDelta.y;
        }
        else if (panV) state.target_pan[1] += ImGui::GetIO().MouseDelta.y;
        else if (panH) state.target_pan[0] += ImGui::GetIO().MouseDelta.x;
    }
}


#endif