#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <chrono>
#include <format>
#include <numeric>
#include <unordered_map>
#include <algorithm>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <implot.h>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "../external/ImGuiFileDialog/ImGuiFileDialog.h"
#define IMGUI_DATEPICKER_YEAR_MIN 1900
#define IMGUI_DATEPICKER_YEAR_MAX 2100
#include "../external/ImGuiDatePicker/ImGuiDatePicker.hpp"

/// DB importttt
#include "lib/db/db.h"

#include "lib/parser/parser.h"

#include "../external/IconFontCppHeaders/IconsMaterialDesign.h"

#define GET_DAY(timePoint) int(timePoint.tm_mday)
#define GET_MONTH_UNSCALED(timePoint) timePoint.tm_mon
#define GET_MONTH(timePoint) int(GET_MONTH_UNSCALED(timePoint) + 1)
#define GET_YEAR(timePoint) int(timePoint.tm_year + 1900)

#define SET_DAY(timePoint, day) timePoint.tm_mday = day
#define SET_MONTH(timePoint, month) timePoint.tm_mon = month - 1
#define SET_YEAR(timePoint, year) timePoint.tm_year = year - 1900

enum class Page
{
    Dashboard,
    Transactions,
    Analytics,
};

struct AppState
{
    // Dashboard Page
    bool dashboardTransactionsLoaded = false;
    std::vector<Transaction> dashboardTransctions;
    double totalSpent = 0.0;
    double spentThisMonth = 0.0;
    double spentLastMonth = 0.0;
    float comparedToLastMonth = 0;
    double balance = 0.0;
    std::vector<Transaction> pendingTransactions;
    bool isPDFparsed = false;

    // Transactions Page
    bool transactionsLoaded = false;
    std::vector<Transaction> transactions;
    std::vector<Transaction> searchTransactions;

    // Analytics Page
    bool analyticsTransactionsLoaded = false;
    std::vector<Transaction> analyticsTransactions;
    std::set<std::string> categories;
    std::vector<int> months; // Ik there are only 12 but this vector includes years too that's why. DONT PREJUDICE.
    std::map<int, std::unordered_map<std::string, double>> monthlySpendings;

    std::string dbError = "Unknown Error xD";

    // FORCE RELOAD, LAZY SHIT
    void forceReloadAllData()
    {
        dashboardTransactionsLoaded = false;
        transactionsLoaded = false;
        analyticsTransactionsLoaded = false;

        // Clear existing data
        dashboardTransctions.clear();
        transactions.clear();
        analyticsTransactions.clear();
        monthlySpendings.clear();
        categories.clear();
        months.clear();
    }
};

// Retuns current month in MMYYYY or MYYYY format
int getCurrMonthYearInt()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm *localTime = std::localtime(&now_c);

    int year = localTime->tm_year + 1900;
    int month = localTime->tm_mon + 1;

    // Always multiply month by 10000 and then adjust for single-digit months
    if (month < 10)
    {
        return month * 10000 + year; // single-digit month, 1 -> 10000
    }
    else
    {
        return month * 10000 + year; // double-digit month, 11 -> 110000 + year
    }
}

void loadDashboardData(AppState &state)
{
    if (!state.dashboardTransactionsLoaded)
    {
        state.dashboardTransctions.clear();
        state.totalSpent = std::stod(getTotalAmountSpentFromDB());
        // state.spentLastMonth = state.totalSpent; // Temporaryyyyyy Dont fuckin forget dumbass!!!!! After you get done with the parser. Update: Got done with Parser now, gotta get the fricking data.

        int thisMonth = getCurrMonthYearInt();
        int lastMonth = ((thisMonth / 10000 + 11) % 12) * 10000 + (thisMonth % 10000 - (thisMonth / 10000 == 1 ? 1 : 0)); // TEMP FIX Because the map only contains the month if there is a txn in db for it. and dont worry about the 10000, you'll get it if you bother to see the values.
        std::cout << thisMonth << std::endl;

        if (state.monthlySpendings.count(thisMonth))

            for (auto &[month, catMap] : state.monthlySpendings)
            {
                std::cout << month << std::endl;
                if (month == thisMonth)
                {
                    for (auto &[cat, amt] : catMap)
                    {
                        state.spentThisMonth += amt;
                        std::cout << "This Month" << amt << std::endl;
                    }
                }
                else if (month == lastMonth)
                {
                    for (auto &[cat, amt] : catMap)
                    {
                        state.spentLastMonth += amt;
                        std::cout << "Last Month" << amt << std::endl;
                    }
                }
            }

        state.comparedToLastMonth = (state.spentThisMonth == 0 || state.spentLastMonth == 0) ? state.spentThisMonth : (state.spentThisMonth / state.spentLastMonth) * 100;
        state.dashboardTransactionsLoaded = true;
    }
}

void loadTransactionsData(AppState &state)
{
    if (!state.transactionsLoaded)
    {
        state.transactions.clear();
        state.transactions = listTransactions();
        state.transactionsLoaded = true;
    }
}

void loadAnalyticsData(AppState &state)
{
    if (!state.analyticsTransactionsLoaded)
    {
        // Clear previous data BEFORE loading new data
        state.analyticsTransactions.clear();
        state.monthlySpendings.clear();
        state.months.clear();
        state.categories.clear();

        state.analyticsTransactions = listTransactions();
        state.monthlySpendings = getMonthlySpendings();

        for (auto &[month, catMap] : state.monthlySpendings)
        {
            for (auto &[cat, amt] : catMap)
            {
                state.categories.insert(cat);
            }

            state.months.push_back(month);
        }

        state.analyticsTransactionsLoaded = true;
    }
}

// I know, will do it later, and make a single func, 2:52AM rn not doing it.
bool validTransactions(std::vector<Transaction> &Txns)
{
    for (auto &t : Txns)
    {
        if (t.amount == 0 || t.method == Transaction::Method::NOTSET || t.method == Transaction::Method::COUNT || t.type == Transaction::TransactionType::NOTSET)
        {
            return false;
        }
    }
    return true;
}

bool validTransaction(Transaction &t)
{

    if (t.amount == 0 || t.method == Transaction::Method::NOTSET || t.method == Transaction::Method::COUNT || t.type == Transaction::TransactionType::NOTSET)
        return false;

    return true;
}

void SetupImGuiStyle()
{
    // Future Dark style by rewrking from ImThemes
    ImGuiStyle &style = ImGui::GetStyle();

    style.Alpha = 1.0f;
    style.DisabledAlpha = 1.0f;
    style.WindowPadding = ImVec2(12.0f, 12.0f);
    style.WindowRounding = 0.0f;
    style.WindowBorderSize = 0.0f;
    style.WindowMinSize = ImVec2(20.0f, 20.0f);
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_None;
    style.ChildRounding = 0.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupRounding = 0.0f;
    style.PopupBorderSize = 1.0f;
    style.FramePadding = ImVec2(6.0f, 6.0f);
    style.FrameRounding = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.ItemSpacing = ImVec2(12.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 3.0f);
    style.CellPadding = ImVec2(12.0f, 6.0f);
    style.IndentSpacing = 20.0f;
    style.ColumnsMinSpacing = 6.0f;
    style.ScrollbarSize = 12.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabMinSize = 12.0f;
    style.GrabRounding = 0.0f;
    style.TabRounding = 0.0f;
    style.TabBorderSize = 0.0f;
    // style.TabMinWidthForCloseButton = 0.0f;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.2745098173618317f, 0.3176470696926117f, 0.4509803950786591f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.5372549295425415f, 0.5529412031173706f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 0.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.15f);     // ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.8f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.173f, 0.173f, 0.180f, 1.0f); // ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 0.8f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.5215686559677124f, 0.6000000238418579f, 0.7019608020782471f, 1.0f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.03921568766236305f, 0.9803921580314636f, 0.9803921580314636f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 0.2901960909366608f, 0.5960784554481506f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.9960784316062927f, 0.4745098054409027f, 0.6980392336845398f, 1.0f);
    style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
    style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
    style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);    // ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
    style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f); // ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.8f); // ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);
}

void DrawBackgroundGradient()
{
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();

    // Single ellipse centered horizontally, positioned at top
    ImVec2 center(windowPos.x + windowSize.x * 0.5f, windowSize.y + 80.0f);

    // Make it very wide horizontally
    float width = windowSize.x * 1.2f;
    float height = 500.0f;

    // Overall opacity for the entire effect
    float overall_opacity = 0.19f;

    // Glow settings
    float glow_strength = 400.0f;
    int glow_steps = 25;

    // Draw glow layers (outermost to innermost)
    for (int i = glow_steps; i > 0; i--)
    {
        float t = (float)i / (float)glow_steps;
        float glow_width = width + t * glow_strength;
        float glow_height = height + t * glow_strength;

        // Very soft white glow
        int alpha = (int)(255 * t * 0.04f * overall_opacity);
        ImU32 glow_color = IM_COL32(255, 255, 255, alpha);

        // Draw as ellipse using proper radius format
        draw_list->AddEllipseFilled(center, ImVec2(glow_width * 0.5f, glow_height * 0.5f), glow_color, 0.0f, 64);
    }

    // Draw main ellipse core
    // int main_alpha = (int)(10 * overall_opacity);
    // ImU32 main_color = IM_COL32(255, 255, 255, main_alpha);
    // draw_list->AddEllipseFilled(center, ImVec2(width * 0.5f, height * 0.5f), main_color, 0.0f, 64);
}

void CenterTextInCol(const char *txt, float posX = ImGui::GetCursorPosX(), ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f))
{
    float colWidth = ImGui::GetColumnWidth();
    float txtWidth = ImGui::CalcTextSize(txt).x;

    float offset = (colWidth - txtWidth) * 0.5f;
    ImGui::SetCursorPosX(posX);
    ImGui::TextColored(color, "%s", txt);
}

void Card(const char *title, double val, const char *postValTxt = "")
{
    float x = ImGui::GetWindowSize().x / 7;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 30.0f);
    ImGui::BeginChild(title, ImVec2(x, 100), true);

    ImGui::Text("%s", title);
    ImGui::Separator();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
    ImGui::Text("%.2f", val);
    ImGui::SameLine();
    ImGui::Text("%s", postValTxt);
    ImGui::PopStyleColor();
    ImGui::EndChild();

    ImGui::PopStyleVar();
}

void DrawHeatMap(AppState &state)
{

    const ImVec2 CELL_SIZE(20.0f, 20.0f);
    const ImVec2 CELL_SPACING(15.0f, 5.0f);
    const float LABEL_MARGIN = 20.0f;
    const float CELL_TEXT_PADDING_X = 4.0f;
    const float CELL_TEXT_PADDING_Y = 2.0f;

    const char *WEEK_DAYS[7] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    const char *WEEK_LABELS[6] = {"Week 1", "Week 2", "Week 3", "Week 4", "Week 5", "Week 6"};

    int currMonth = getCurrMonthYearInt();
    int year = currMonth % 10000;
    int month = currMonth / 10000;

    int firstDayOfMonth = ImGui::DayOfWeek(1, month, year);
    int numDaysInMonth = ImGui::NumDaysInMonth(month, year);
    int numWeeksInMonth = ImGui::NumWeeksInMonth(month, year);

    std::vector<double> txnOfMonth(numDaysInMonth, 0.0);
    if (!state.transactions.empty())
    {
        for (auto &t : state.transactions)
        {
            std::tm tm = {};
            std::istringstream ss(t.date);
            ss >> std::get_time(&tm, "%Y-%m-%d");

            if (ss && (tm.tm_year + 1900 == year) && (tm.tm_mon + 1 == month) &&
                t.type == Transaction::TransactionType::Expense)
            {
                txnOfMonth[tm.tm_mday - 1] += t.amount;
            }
        }
    }

    double maxTxn = *std::max_element(txnOfMonth.begin(), txnOfMonth.end());
    if (maxTxn <= 0.0)
        maxTxn = 1.0; // CUZ SHE WASN'T THE ONE xD

    auto normalize = [&](double v)
    {
        if (v <= 0.0)
            return 0.0;
        return std::log10(v + 1.0) / std::log10(maxTxn + 1.0);
    };

    // COLOR LEVELS IN CASE YOU FORGET(white -> light red -> deep red), THERE MORE THAN 1 RED APPRENTLY
    auto getColor = [&](double v)
    {
        double t = normalize(v); 
        int r = 255;
        int g = (int)(255 * (1.0 - t));
        int b = (int)(255 * (1.0 - t));
        return IM_COL32(r, g, b, 255);
    };

    ImVec2 start = ImGui::GetCursorScreenPos();
    float labelWidth = ImGui::CalcTextSize("Week 1").x;
    float labelOffset = labelWidth + LABEL_MARGIN;

    ImDrawList *drawList = ImGui::GetWindowDrawList();

    std::string header = "Monthly Spendings Heatmap";
    float totalWidth = labelOffset + 7 * (CELL_SIZE.x + CELL_SPACING.x);
    float headerX = start.x + (totalWidth - ImGui::CalcTextSize(header.c_str()).x) * 0.5f;
    drawList->AddText(ImVec2(headerX, start.y), IM_COL32(255, 255, 255, 255), header.c_str());

    start.y += ImGui::CalcTextSize(header.c_str()).y + CELL_SPACING.y * 2;
    drawList->AddLine(ImVec2(start.x, start.y),
                      ImVec2(start.x + totalWidth, start.y),
                      IM_COL32(255, 255, 255, 255), 1.0f);
    start.y += 5;

    for (int j = 0; j < 7; j++)
    {
        ImVec2 pos(start.x + labelOffset + j * (CELL_SIZE.x + CELL_SPACING.x), start.y);
        drawList->AddText(pos, IM_COL32(255, 255, 255, 255), WEEK_DAYS[j]);
    }

    start.y += ImGui::GetTextLineHeightWithSpacing() + CELL_SPACING.y;

    for (int week = 0; week < numWeeksInMonth; week++)
    {
        std::vector<int> calWeek = ImGui::CalendarWeek(week + 1, firstDayOfMonth, numDaysInMonth);

        ImVec2 labelPos(start.x, start.y + week * (CELL_SIZE.y + CELL_SPACING.y));
        drawList->AddText(labelPos, IM_COL32(255, 255, 255, 255), WEEK_LABELS[week]);

        for (int j = 0; j < 7; j++)
        {
            int day = calWeek[j];
            if (day != 0)
            {
                ImVec2 posMin(start.x + labelOffset + j * (CELL_SIZE.x + CELL_SPACING.x),
                              start.y + week * (CELL_SIZE.y + CELL_SPACING.y));
                ImVec2 posMax(posMin.x + CELL_SIZE.x, posMin.y + CELL_SIZE.y);

                double val = txnOfMonth[day - 1];
                ImU32 color = getColor(val);

                drawList->AddRectFilled(posMin, posMax, color, 4.0f);

                char buf[4];
                snprintf(buf, sizeof(buf), "%d", day);
                drawList->AddText(ImVec2(posMin.x + CELL_TEXT_PADDING_X,
                                         posMin.y + CELL_TEXT_PADDING_Y),
                                  IM_COL32(0, 0, 0, 255), buf);

                if (ImGui::IsMouseHoveringRect(posMin, posMax))
                {
                    ImGui::BeginTooltip();
                    ImGui::Text("Day %d\nSpent: %.2f", day, val);
                    ImGui::EndTooltip();
                }
            }
        }
    }

    ImVec2 total_size(totalWidth,
                      numWeeksInMonth * (CELL_SIZE.y + CELL_SPACING.y) - CELL_SPACING.y);
    ImGui::Dummy(total_size);
}

bool dashboardOpened = true;
int windowSizeX = 1920;
int windowSizeY = 1080;
Page currPage = Page::Dashboard; // Spawn Point

int main()
{
    dotenv::init("../app/.env");

    std::string host = std::getenv("DB_HOST");
    std::string dbName = std::getenv("DB_NAME");
    std::string user = std::getenv("DB_USER");
    std::string password = std::getenv("DB_PASSWORD");

    PGconn *conn = connectDB(host, dbName, user, password);
    AppState state;

    loadAnalyticsData(state);
    loadDashboardData(state);
    loadTransactionsData(state);

    // Yea what?? Just creating a window.
    if (!glfwInit())
        return -1;
    GLFWwindow *window = glfwCreateWindow(windowSizeX, windowSizeY, "Coinflict", nullptr, nullptr);
    if (!window)
        return -1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    SetupImGuiStyle();

    float fontSize = 18.0f;
    ImGuiIO &io = ImGui::GetIO();
    ImFont *poppinsMediumItalic = io.Fonts->AddFontFromFileTTF("../app/lib/assets/Poppins-MediumItalic.ttf", fontSize);
    ImFont *poppinsRegular = io.Fonts->AddFontFromFileTTF("../app/lib/assets/Poppins-Regular.ttf", fontSize);

    io.FontDefault = poppinsRegular;
    (void)io;

    float baseFontSize = 13.0f; // 13.0f is the size of the default font. Change to the font size you use.
    float iconFontSize = baseFontSize * 2.0f / 3.0f;

    static const ImWchar icons_ranges[] = {ICON_MIN_MD, 0xFFFF, 0};
    ImFontConfig iconsConfig;
    iconsConfig.MergeMode = true; // IT WILL MERGE INTO THE LAST LOADED FONT.
    iconsConfig.PixelSnapH = true;
    iconsConfig.GlyphMinAdvanceX = iconFontSize;
    iconsConfig.GlyphOffset = ImVec2(0, 5);
    io.Fonts->AddFontFromFileTTF("../app/lib/assets/MaterialIcons-Regular.ttf", 18.0f, &iconsConfig, icons_ranges);

    // io.Fonts->Build();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
                                        ImGuiWindowFlags_NoResize |
                                        ImGuiWindowFlags_NoMove |
                                        ImGuiWindowFlags_NoCollapse |
                                        ImGuiWindowFlags_NoBringToFrontOnFocus |
                                        ImGuiWindowFlags_NoNavFocus;

        ImGui::Begin("Home Menu", &dashboardOpened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
        DrawBackgroundGradient();

        const char *tabs[] = {"Dashboard", "Transactions", "Analytics"};
        float tabWidth = ImGui::GetContentRegionAvail().x / IM_ARRAYSIZE(tabs);

        for (int i = 0; i < 3; i++)
        {

            if (i > 0)
                ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            if (ImGui::Button(tabs[i], ImVec2(tabWidth, 0)))
            {
                currPage = static_cast<Page>(i); // Update current page
            }
            ImGui::PopStyleColor();

            static float hoverAnimT[3] = {0.0f, 0.0f, 0.0f};
            float dt = ImGui::GetIO().DeltaTime;
            float speed = 3.0f;
            ImVec2 p1 = ImGui::GetItemRectMin();
            ImVec2 p2 = ImGui::GetItemRectMax();
            ImVec2 lineY(p1.x, p2.y);
            float inset = 150.0f;
            float lineHalf = (p2.x - p1.x) / 2.0f - inset;

            if (currPage == static_cast<Page>(i))
            {
                ImDrawList *drawList = ImGui::GetWindowDrawList();
                ImVec2 start(p1.x + (p2.x - p1.x) / 2.0f - lineHalf, p2.y);
                ImVec2 end(p1.x + (p2.x - p1.x) / 2.0f + lineHalf, p2.y);

                drawList->AddLine(start, end, IM_COL32(255, 255, 255, 150), 1.0f);
            }
            if (ImGui::IsItemHovered())
            {
                hoverAnimT[i] += dt * speed;
                if (hoverAnimT[i] > 1.0f)
                    hoverAnimT[i] = 1.0f;
                /*ImDrawList *draw_list = ImGui::GetWindowDrawList();
                ImVec2 pos = ImGui::GetItemRectMin();
                ImVec2 size = ImGui::GetItemRectSize();
                ImVec2 p1 = ImGui::GetItemRectMin();
                ImVec2 p2 = ImGui::GetItemRectMax();


                ImVec2 lineStart(p1.x, p2.y); // Bottom-left
                ImVec2 lineEnd(p2.x, p2.y);   // Bottom-right

                draw_list->AddLine(lineStart, lineEnd, IM_COL32(255, 255, 255, 255), 1.0f); */
            }
            else
            {
                hoverAnimT[i] -= dt * speed;
                if (hoverAnimT[i] < 0.0f)
                    hoverAnimT[i] = 0.0f;
            }

            float animatedHalf = lineHalf * hoverAnimT[i];
            ImVec2 start(p1.x + (p2.x - p1.x) / 2.0f - animatedHalf, p2.y);
            ImVec2 end(p1.x + (p2.x - p1.x) / 2.0f + animatedHalf, p2.y);

            if (hoverAnimT[i] > 0.0f)
            {

                ImDrawList *drawList = ImGui::GetWindowDrawList();
                drawList->AddLine(start, end, IM_COL32(255, 255, 255, 150), 1.0f);
            }
        }

        switch (currPage)
        {
        case Page::Dashboard:
        {
            loadDashboardData(state);

            // --- DASHBOARD WINDOW ---
            ImGui::BeginGroup();

            Card("Total Spent", state.totalSpent);
            ImGui::SameLine();
            Card("This Month", state.spentThisMonth);
            ImGui::SameLine();
            Card("Balance", state.balance);
            ImGui::SameLine();
            Card("Compared To Last Month", state.comparedToLastMonth, "%");

            ImGui::SameLine();
            // THE PDF SELECTOR FOR THE USER.  (do it manually later instead of the dependencies)
            if (ImGui::Button(ICON_MD_CLOUD_UPLOAD " Upload Statement", ImVec2(ImGui::GetWindowSize().x / 7, 100)))
            {
                std::cout << "clicked" << std::endl;
                IGFD::FileDialogConfig config;
                config.path = ".";

                // Open the dialog (key, title, filter, path)
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose a PDF", ".pdf", config);
                state.pendingTransactions.clear();
            }

            std::string invalidPDF = "##invalidPDF";
            if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize, ImVec2{(800.0f), (600.0f)}))
            {

                if (ImGuiFileDialog::Instance()->IsOk())
                {
                    std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                    // Do something with filePath
                    std::cout << "User selected: " << filePath << std::endl;
                    state.pendingTransactions = parsePDF(filePath);
                    if (state.pendingTransactions.empty())
                    {
                        state.isPDFparsed = false;
                        ImGui::OpenPopup(invalidPDF.c_str());
                    }
                    state.isPDFparsed = true;
                }
                ImGuiFileDialog::Instance()->Close();
            }
            if (ImGui::BeginPopupModal(invalidPDF.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: Invalid PDF. \nThe PDF either contains no transaction records or is completely invalid.");
                if (ImGui::Button("Okay"))
                {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            static bool newTxnPopup = false;
            std::string newTxnpopupId = "##NewTxn";
            std::vector<std::string> catVec(state.categories.begin(), state.categories.end());

            if (state.isPDFparsed)
            {

                // TODO: ADD DEFAULT MESSAGES LIKE NO TRANSACTIONS FOUND IN PDF ETC ETC................
                if (ImGui::BeginTable("ParsedTransactions", 6))
                {
                    ImGui::TableSetupColumn("Category");
                    ImGui::TableSetupColumn("Amount");
                    ImGui::TableSetupColumn("Date");
                    ImGui::TableSetupColumn("Label");
                    ImGui::TableSetupColumn("Method");
                    ImGui::TableSetupColumn("Select");

                    ImGui::TableHeadersRow();

                    static std::vector<uint8_t> rowSelected;
                    static std::vector<bool> openNewCategoryPopup;
                    static std::string multiSelectCategory;
                    if (rowSelected.size() != state.pendingTransactions.size())
                        rowSelected.assign(state.pendingTransactions.size(), 0);
                    if (openNewCategoryPopup.size() != state.pendingTransactions.size())
                        openNewCategoryPopup.assign(state.pendingTransactions.size(), false);
                    for (size_t i = 0; i < state.pendingTransactions.size(); i++)
                    {
                        Transaction &t = state.pendingTransactions[i];

                        ImGui::TableNextRow();
                        ImVec4 amountColor = (t.type == Transaction::TransactionType::Expense) ? ImVec4(1.0f, 0.0f, 0.0f, 1.0f) : ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::TextColored(amountColor, "%.2f", t.amount);

                        ImGui::TableSetColumnIndex(0);

                        std::string comboId = "##Category" + std::to_string(i);
                        std::string popupId = "##NewCategory" + std::to_string(i);
                        std::string newCatLabel = "+New";
                        if (ImGui::BeginCombo(comboId.c_str(), t.category.c_str()))
                        {
                            for (int j = 0; j < state.categories.size(); j++)
                            {
                                const char *c = catVec[j].c_str();
                                bool isSelected = (t.category == c);
                                if (ImGui::Selectable(c, isSelected))
                                {
                                    t.category = std::string(c);

                                    if (rowSelected[i])
                                    {
                                        for (size_t k = 0; k < state.pendingTransactions.size(); k++)
                                        {
                                            if (k != i && rowSelected[k])
                                            {
                                                state.pendingTransactions[k].category = t.category;
                                            }
                                        }
                                    }
                                }
                                if (isSelected)
                                {
                                    ImGui::SetItemDefaultFocus();
                                }
                            }

                            if (ImGui::Selectable(newCatLabel.c_str()))
                            {
                                openNewCategoryPopup[i] = true;

                                std::cout << "POP UP" << std::endl;
                            }
                            ImGui::EndCombo();
                        }
                        if (openNewCategoryPopup[i])
                        {
                            ImGui::OpenPopup(popupId.c_str());
                            openNewCategoryPopup[i] = false;
                        }
                        // POP UP CODE. TODO: MAKE IT MORE POPUP'ish
                        if (ImGui::BeginPopupModal(popupId.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                        {
                            static char newCategoryBuf[64] = "";
                            ImGui::InputText("Name", newCategoryBuf, IM_ARRAYSIZE(newCategoryBuf));
                            if (ImGui::Button("Add"))
                            {
                                if (strlen(newCategoryBuf) > 0)
                                {
                                    state.categories.insert(newCategoryBuf);
                                    t.category = newCategoryBuf; // assign to this transaction
                                    newCategoryBuf[0] = '\0';
                                    ImGui::CloseCurrentPopup();
                                }
                            }
                            ImGui::SameLine();
                            if (ImGui::Button("Cancel"))
                            {
                                newCategoryBuf[0] = '\0';
                                ImGui::CloseCurrentPopup();
                            }
                            ImGui::EndPopup();
                        }

                        ImGui::TableSetColumnIndex(2);

                        ImGui::Text("%s", t.date.c_str());

                        ImGui::TableSetColumnIndex(3);

                        static char buff[128];
                        strncpy(buff, t.label.c_str(), sizeof(buff));
                        if (ImGui::InputText(("##label" + std::to_string(i)).c_str(), buff, IM_ARRAYSIZE(buff)))
                        {
                            t.label = buff;
                            if (rowSelected[i])
                            {
                                for (size_t j = 0; j < state.pendingTransactions.size(); j++)
                                {
                                    if (rowSelected[j] && i != j)
                                    {
                                        state.pendingTransactions[j].label = buff;
                                    }
                                }
                            }
                        }

                        ImGui::TableSetColumnIndex(4);
                        std::string comboLabel = "##Method" + std::to_string(i);
                        if (ImGui::BeginCombo(comboLabel.c_str(), Transaction::toString(t.method).c_str()))
                        {
                            for (int j = 0; j < static_cast<int>(Transaction::Method::COUNT); j++)
                            {
                                Transaction::Method m = static_cast<Transaction::Method>(j);
                                bool isSelected = (t.method == m);

                                if (ImGui::Selectable(Transaction::toString(m).c_str(), isSelected))
                                {

                                    t.method = m;

                                    if (rowSelected[i])
                                    {
                                        for (size_t j = 0; j < state.pendingTransactions.size(); j++)
                                        {
                                            if (rowSelected[j] && i != j)
                                            {
                                                state.pendingTransactions[j].method = t.method;
                                            }
                                        }
                                    }
                                }

                                if (isSelected)
                                {
                                    ImGui::SetItemDefaultFocus();
                                }
                            }
                            ImGui::EndCombo();
                        }

                        // TODO: ADD EASE OF LIFE STUFF LIKE CLEAR BUTTON, ENTIRE COLUMN SELECTION ETC ETC......
                        ImGui::TableSetColumnIndex(5);

                        std::string checkboxId = "##checkbox" + std::to_string(i);
                        if (ImGui::Checkbox(checkboxId.c_str(), (bool *)&rowSelected[i]))
                        {
                            std::cout << "Checkbox for row: " << i << " is checked." << (bool)rowSelected[i] << std::endl;
                        }
                    }

                    ImGui::EndTable();
                }

                bool saveConfirmationPopup = false;
                bool invalidTransactionPopup = false;
                std::string confirmationPopupId = "ConfirmEdits##";
                std::string invalidTxnsPopupId = "InvalidTxns##";

                if (ImGui::Button("Save"))
                {
                    if (validTransactions(state.pendingTransactions))
                    {
                        saveConfirmationPopup = true;
                    }
                    else
                    {
                        invalidTransactionPopup = true;
                    }
                }

                if (saveConfirmationPopup)
                {
                    ImGui::OpenPopup(confirmationPopupId.c_str());
                    saveConfirmationPopup = false;
                }
                if (invalidTransactionPopup)
                {
                    ImGui::OpenPopup(invalidTxnsPopupId.c_str());
                    invalidTransactionPopup = false;
                }

                if (ImGui::BeginPopupModal(confirmationPopupId.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    static bool confirmed = false;
                    static bool closeConfirmationPopup = false;
                    static bool actionResultPopup = false;
                    static std::string actionResultPopupId = "##TxnResult";
                    static std::vector<bool> txnInsert;
                    static bool allSuccess = false;

                    if (!confirmed)
                    {
                        ImGui::Text("Double Check");
                        if (ImGui::BeginTable("ConfirmationTable", 5))
                        {
                            ImGui::TableSetupColumn("Category");
                            ImGui::TableSetupColumn("Amount");
                            ImGui::TableSetupColumn("Date");
                            ImGui::TableSetupColumn("Label");
                            ImGui::TableSetupColumn("Method");

                            ImGui::TableHeadersRow();

                            for (auto &t : state.pendingTransactions)
                            {

                                ImGui::TableNextRow();

                                ImGui::TableSetColumnIndex(0);
                                ImGui::Text("%s", t.category.c_str());

                                ImGui::TableSetColumnIndex(1);
                                ImVec4 amountColor = (t.type == Transaction::TransactionType::Expense) ? ImVec4(1.0f, 0.0f, 0.0f, 1.0f) : ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
                                ImGui::TextColored(amountColor, "%.2f", t.amount);

                                ImGui::TableSetColumnIndex(2);
                                ImGui::Text("%s", t.date.c_str());

                                ImGui::TableSetColumnIndex(3);
                                ImGui::Text("%s", t.label.c_str());

                                ImGui::TableSetColumnIndex(4);
                                ImGui::Text("%s", Transaction::toString(t.method).c_str());
                            }
                            ImGui::EndTable();
                        }
                        if (ImGui::Button("Confirm"))
                        {
                            confirmed = true;
                            txnInsert.clear();
                            txnInsert.resize(state.pendingTransactions.size(), false);

                            std::cout << "--------------------Saving To DB------------------------" << std::endl;
                            for (size_t i = 0; i < state.pendingTransactions.size(); i++)
                            {
                                Transaction &t = state.pendingTransactions[i];
                                txnInsert[i] = addTransactionToDB(t.amount, t.label, t.category, Transaction::toString(t.method), t.type, t.date);
                            }

                            allSuccess = std::all_of(txnInsert.begin(), txnInsert.end(), [](bool b)
                                                     { return b; });
                            actionResultPopup = true;
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Cancel"))
                        {
                            confirmed = false;
                            ImGui::CloseCurrentPopup();
                        }
                    }

                    if (actionResultPopup)
                    {
                        ImGui::OpenPopup(actionResultPopupId.c_str());
                        actionResultPopup = false;
                    }

                    // The POP UP THAT SHOWS THE RESULT OF THE QUERY.
                    if (ImGui::BeginPopupModal(actionResultPopupId.c_str()))
                    {
                        if (allSuccess)
                        {
                            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Transactions added successfully!");
                        }
                        else
                        {
                            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: Transactions could not be added.");
                        }

                        if (ImGui::Button("Okay"))
                        {
                            if (allSuccess)
                            {
                                state.pendingTransactions.clear();
                                state.isPDFparsed = false;

                                // FORCE RELOAD ALL DATA
                                state.forceReloadAllData();
                            }

                            confirmed = false;
                            closeConfirmationPopup = true;
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }

                    if (closeConfirmationPopup)
                    {
                        ImGui::CloseCurrentPopup();
                        closeConfirmationPopup = false;
                    }

                    ImGui::EndPopup();
                }

                if (ImGui::BeginPopupModal(invalidTxnsPopupId.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: One or more transactions has invalid properties.");
                    if (ImGui::Button("Okay"))
                    {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
            }

            ImGui::SameLine();

            if (!state.isPDFparsed)
            {
                if (ImGui::Button(ICON_MD_ADD_CIRCLE_OUTLINE " New Transaction", ImVec2(ImGui::GetWindowSize().x / 7, 100)))
                {
                    newTxnPopup = true;
                }
            }

            if (newTxnPopup)
            {
                ImVec2 pos = ImVec2(ImGui::GetWindowSize().x / 3.0f, ImGui::GetWindowSize().y / 3.0f);
                ImGui::SetNextWindowPos(pos);
                ImGui::OpenPopup(newTxnpopupId.c_str());
                newTxnPopup = false;
            }

            static Transaction t;
            static bool addNewTxn = false;
            static double amt = 0;
            static char labelBuf[128] = "";
            static std::string strTxnDate;

            static std::string actionResultPopupId = "##TxnResult";
            static std::string invalidTxnsPopupId = "##InvalidTxn";
            if (ImGui::BeginPopupModal(newTxnpopupId.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                // ASK FOR amount, category, label, method, type(income/expense), date(default today)

                ImGui::Text("Amount: ");
                if (ImGui::InputDouble("##amt", &amt, 0.0, 0.0, "%.2f"))
                {
                    t.amount = amt;
                    // std::cout << "New Amount: " << amt << std::endl;
                }

                ImGui::Text("Category: ");
                std::string popupId = "##NewCategory";
                std::string newCatLabel = "+New";
                static bool newCatPopup = false;

                if (ImGui::BeginCombo("##NewCategory", t.category.c_str()))
                {
                    for (int i = 0; i < state.categories.size(); i++)
                    {
                        const char *c = catVec[i].c_str();
                        bool isCatSelected = (t.category == c);

                        if (ImGui::Selectable(c, isCatSelected))
                        {
                            t.category = std::string(c);
                            // std::cout << c << " " << t.category << std::endl;
                        }

                        if (isCatSelected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }

                    if (ImGui::Selectable(newCatLabel.c_str()))
                    {
                        newCatPopup = true;
                        // std::cout << "POP UP" << std::endl;
                    }
                    ImGui::EndCombo();
                }
                if (newCatPopup)
                {
                    ImGui::OpenPopup(popupId.c_str());
                    newCatPopup = false;
                }
                // POP UP CODE. TODO: MAKE IT MORE POPUP'ish
                if (ImGui::BeginPopupModal(popupId.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    static char newCategoryBuf[64] = "";
                    ImGui::InputText("##NewCat", newCategoryBuf, IM_ARRAYSIZE(newCategoryBuf));
                    if (ImGui::Button("Add"))
                    {
                        if (strlen(newCategoryBuf) > 0)
                        {
                            state.categories.insert(newCategoryBuf);
                            t.category = newCategoryBuf; // assign to this transaction
                            newCategoryBuf[0] = '\0';
                            ImGui::CloseCurrentPopup();
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel"))
                    {
                        newCategoryBuf[0] = '\0';
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                ImGui::Text("Label: ");
                if (ImGui::InputText("##label", labelBuf, IM_ARRAYSIZE(labelBuf)))
                {
                    std::cout << "New Label: " << labelBuf << std::endl;
                    t.label = labelBuf;
                }

                ImGui::Text("Method: ");
                if (ImGui::BeginCombo("##NewMethod", Transaction::toString(t.method).c_str()))
                {

                    for (size_t i = 0; i < static_cast<int>(Transaction::Method::COUNT); i++)
                    {

                        Transaction::Method m = static_cast<Transaction::Method>(i);
                        bool isSelected = (m == t.method);

                        if (ImGui::Selectable(Transaction::toString(m).c_str(), &isSelected))
                        {
                            t.method = m;
                        }

                        if (isSelected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                ImGui::Text("Type: ");
                if (ImGui::BeginCombo("##Type", Transaction::toString(t.type).c_str()))
                {

                    for (size_t i = 0; i < static_cast<int>(Transaction::TransactionType::COUNT); i++)
                    {

                        Transaction::TransactionType type = static_cast<Transaction::TransactionType>(i);
                        bool isSelected = (t.type == type);

                        if (ImGui::Selectable(Transaction::toString(type).c_str(), &isSelected))
                        {
                            t.type = type;
                        }
                        if (isSelected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }

                    ImGui::EndCombo();
                }

                // NEED TO RESET THE STYLES FOR THE DATE PICKER SINCE THE DATE PICKER STYLES WERE ALSO GETTING RESET DUE TO MY UI-STYLES.
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 3));
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 2));
                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 2));

                static std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
                static std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
                static tm ti = *std::gmtime(&currentTime);
                if (ImGui::DatePickerEx("Date: ", ti, poppinsRegular))
                {
                    std::ostringstream oss;
                    oss << std::put_time(&ti, "%d-%m-%Y");
                    strTxnDate = oss.str();

                    // std::cout << "txnDate: " << strTxnDate << std::endl;
                    t.date = strTxnDate;
                }
                ImGui::PopStyleVar(3);
                // TODO: FIX -> IS RUNNING EVERY FRAME.

                std::ostringstream oss;
                oss << std::put_time(&ti, "%d-%m-%Y");
                strTxnDate = oss.str();

                std::cout << "txnDate: " << strTxnDate << std::endl;
                t.date = strTxnDate;

                if (ImGui::Button("Save"))
                {
                    std::cout << t.amount << " " << Transaction::toString(t.method) << "  " << Transaction::toString(t.type) << std::endl;
                    if (validTransaction(t))
                    {
                        addNewTxn = addTransactionToDB(t.amount, t.label, Transaction::toString(t.method), t.category, t.type, t.date);
                        ImGui::OpenPopup(actionResultPopupId.c_str());
                        std::cout << addNewTxn << std::endl;

                        if (addNewTxn)
                        {
                            std::cout << " Added new Transaction to DB!" << std::endl;
                        }
                        else
                        {
                            ImGui::OpenPopup(invalidTxnsPopupId.c_str());
                        }
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel"))
                {
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::BeginPopupModal(actionResultPopupId.c_str()))
                {
                    if (addNewTxn)
                    {
                        ImGui::TextColored(ImVec4(0, 1, 0, 1), "Transactions added successfully!");
                    }
                    else
                    {
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: Transactions could not be added.");
                    }

                    if (ImGui::Button("Okay"))
                    {
                        if (addNewTxn)
                        {
                            state.pendingTransactions.clear();

                            // FORCE RELOAD ALL DATA
                            state.forceReloadAllData();
                        }

                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                if (ImGui::BeginPopupModal(invalidTxnsPopupId.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: Transaction has invalid properties.");
                    if (ImGui::Button("Okay"))
                    {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
                ImGui::EndPopup();
            }

            ImGui::EndGroup();
            // Summary row
            ImGui::Text("Last few transaction");
            ImGui::Separator();
            ImGui::Spacing();

            loadTransactionsData(state);
            ImGui::Text("Expenses");
            static float posX[5];
            static float colAnchor[5];

            if (state.transactions.empty())
            {
                ImGui::PushFont(poppinsMediumItalic);
                ImGui::TextColored(ImVec4(1.0f, 0.422f, 0.422f, 1.0f), "%s", "NO TRANSACTIONS FOUND IN THE DATABASE! xD");
                ImGui::PopFont();
            }
            else
            {
                if (ImGui::BeginTable("ExpensesTable", 5, ImGuiTableFlags_NoBordersInBody))
                {
                    ImGui::TableNextRow();
                    const char *headers[] = {"Date", "Category", "Label", "Amount", "Method"};

                    for (int col = 0; col < IM_ARRAYSIZE(headers); col++)
                    {
                        ImGui::TableSetColumnIndex(col);

                        const char *text = headers[col];
                        float colWidth = ImGui::GetColumnWidth();
                        float txtWidth = ImGui::CalcTextSize(text).x;
                        float offsetX = (colWidth - txtWidth) * 0.5f;
                        posX[col] = ImGui::GetCursorPosX() + offsetX;

                        ImGui::SetCursorPosX(posX[col]);
                        ImGui::TableHeader(text);
                        colAnchor[col] = posX[col] + txtWidth;
                    }

                    /*ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_IndentEnable);
                    ImGui::TableSetupColumn("Amount");
                    ImGui::TableSetupColumn("Date");
                    ImGui::TableHeadersRow(); */

                    for (int i = 0; i < 20; i++)
                    {
                        float txtWidth;
                        auto txn = state.transactions[i];

                        ImGui::TableNextRow();

                        ImGui::TableSetColumnIndex(0);
                        txtWidth = ImGui::CalcTextSize(txn.date.c_str()).x;
                        CenterTextInCol(txn.date.c_str(), colAnchor[0] - txtWidth);

                        ImGui::TableSetColumnIndex(1);
                        txtWidth = ImGui::CalcTextSize(txn.category.c_str()).x;
                        CenterTextInCol(txn.category.c_str(), colAnchor[1] - txtWidth);

                        ImGui::TableSetColumnIndex(2);
                        txtWidth = ImGui::CalcTextSize(txn.label.c_str()).x;
                        CenterTextInCol(txn.label.c_str(), colAnchor[2] - txtWidth);

                        ImGui::TableSetColumnIndex(3);
                        std::string strAmt = std::format("{:.2f}", txn.amount);
                        ImVec4 amountColor = (txn.type == Transaction::TransactionType::Expense) ? ImVec4(1.000f, 0.322f, 0.322f, 1.0f) : ImVec4(0.000f, 0.784f, 0.325f, 1.0f);
                        txtWidth = ImGui::CalcTextSize(strAmt.c_str()).x;
                        CenterTextInCol(strAmt.c_str(), colAnchor[3] - txtWidth, amountColor);

                        ImGui::TableSetColumnIndex(4);
                        std::string strMethod = Transaction::toString(txn.method);
                        txtWidth = ImGui::CalcTextSize(strMethod.c_str()).x;
                        CenterTextInCol(strMethod.c_str(), colAnchor[4] - txtWidth);
                    }
                    ImGui::EndTable();
                }
            }
            ImGui::Separator();

            break;
        }

        case Page::Transactions:
        {
            // Ensure transactions data is loaded
            loadTransactionsData(state);

            ImGui::BeginTabBar("TransactionsBar");

            if (ImGui::BeginTabItem("Recent Transactions"))
            {

                if (state.transactions.empty())
                {
                    ImGui::PushFont(poppinsMediumItalic);
                    ImGui::TextColored(ImVec4(1.000f, 0.422f, 0.422f, 1.0f), "%s", "NO TRANSACTIONS IN THE DATABSE! xD");
                    ImGui::PopFont();
                }
                else
                {
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                    if (ImGui::BeginTable("RecentTransactions", 5, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_ScrollY))
                    {
                        ImGui::TableNextRow();
                        const char *headers[] = {"Date", "Category", "Label", "Amount", "Method"}; // TODO: FIX THIS AND MAKEIT BETTER. PROPER FORMATING AND SHOW ALL INFO. THAT IS FOR OTHER TBLES TOO.
                        static float posX[5];
                        static float colAnchor[5];
                        for (int col = 0; col < IM_ARRAYSIZE(headers); col++)
                        {
                            ImGui::TableSetColumnIndex(col);

                            const char *text = headers[col];
                            float colWidth = ImGui::GetColumnWidth();
                            float txtWidth = ImGui::CalcTextSize(text).x;
                            float offsetX = (colWidth - txtWidth) * 0.5f;
                            posX[col] = ImGui::GetCursorPosX() + offsetX;

                            ImGui::SetCursorPosX(posX[col]);
                            ImGui::TableHeader(text);
                            colAnchor[col] = posX[col] + txtWidth;
                        }

                        for (auto &t : state.transactions)
                        {
                            float txtWidth;

                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            txtWidth = ImGui::CalcTextSize(t.date.c_str()).x;
                            CenterTextInCol(t.date.c_str(), colAnchor[0] - txtWidth);

                            ImGui::TableSetColumnIndex(1);
                            txtWidth = ImGui::CalcTextSize(t.category.c_str()).x;
                            CenterTextInCol(t.category.c_str(), colAnchor[1] - txtWidth);

                            ImGui::TableSetColumnIndex(2);
                            txtWidth = ImGui::CalcTextSize(t.label.c_str()).x;
                            CenterTextInCol(t.label.c_str(), colAnchor[2] - txtWidth);

                            ImGui::TableSetColumnIndex(3);
                            std::string strAmt = std::format("{:.2f}", t.amount);
                            ImVec4 amountColor = (t.type == Transaction::TransactionType::Expense) ? ImVec4(1.000f, 0.322f, 0.322f, 1.0f) : ImVec4(0.000f, 0.784f, 0.325f, 1.0f);
                            txtWidth = ImGui::CalcTextSize(strAmt.c_str()).x;
                            CenterTextInCol(strAmt.c_str(), colAnchor[3] - txtWidth, amountColor);

                            ImGui::TableSetColumnIndex(4);
                            std::string strMethod = Transaction::toString(t.method);
                            txtWidth = ImGui::CalcTextSize(strMethod.c_str()).x;
                            CenterTextInCol(strMethod.c_str(), colAnchor[4] - txtWidth);
                        }

                        ImGui::EndTable();
                    }
                    ImGui::PopStyleColor();
                }

                ImGui::EndTabItem();
            }

            float fullWidth = ImGui::GetContentRegionAvail().x;
            float primaryWidth = fullWidth * 0.5f * 0.3f;
            float secondaryWidth = fullWidth * 0.5f * 0.2f;

            if (ImGui::BeginTabItem(ICON_MD_SEARCH " Search Transaction"))
            {

                static std::string properties[] = {"Amount", "Label", "Category", "Date", "Method", "Type"};
                static std::string searchProp = "Label";
                static char searchBuf[128];
                static std::string searchVal;
                ImGui::SetNextItemWidth(primaryWidth);
                if (ImGui::InputText("##searchTxns", searchBuf, IM_ARRAYSIZE(searchBuf), ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll))
                {
                    searchVal = std::string(searchBuf);
                    state.searchTransactions = searchTransactions(searchProp, searchVal);
                    std::cout << "Search for: " << searchVal << " in: " << searchProp << std::endl;
                }
                ImGui::SameLine();

                ImVec2 p1 = ImGui::GetItemRectMin();
                ImVec2 p2 = ImGui::GetItemRectMax();
                float buttonSize = p2.y - p1.y;
                ImGui::SetCursorScreenPos(ImVec2(p2.x, p1.y));
                if (ImGui::Button(ICON_MD_CLEAR, ImVec2(buttonSize, buttonSize)))
                {
                    std::cout << "CLEAR!" << std::endl;
                    state.searchTransactions.clear();

                    searchVal = "";
                    memset(searchBuf, 0, sizeof(searchBuf));
                }
                ImGui::SameLine();

                ImGui::SetNextItemWidth(secondaryWidth);
                if (ImGui::BeginCombo("##searchBy", searchProp.c_str(), ImGuiComboFlags_PopupAlignLeft))
                {

                    for (size_t i = 0; i < IM_ARRAYSIZE(properties); i++)
                    {
                        const char *p = properties[i].c_str();
                        bool isSelected = (p == searchProp);
                        if (ImGui::Selectable(properties[i].c_str(), isSelected))
                        {
                            searchProp = p;
                        }
                        if (isSelected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }

                    ImGui::EndCombo();
                }

                if (ImGui::Button(ICON_MD_SEARCH " Search"))
                {
                    state.searchTransactions = searchTransactions(searchProp, searchVal);
                    std::cout << "Search for: " << searchVal << " in: " << searchProp << std::endl;
                }

                if (state.searchTransactions.empty())
                {
                    ImGui::Text("No Transactions To Show!");
                }
                else
                {
                    if (ImGui::BeginTable("##searchRes", 6))
                    {
                        ImGui::TableSetupColumn("Category");
                        ImGui::TableSetupColumn("Label");
                        ImGui::TableSetupColumn("Amount");
                        ImGui::TableSetupColumn("Method");
                        ImGui::TableSetupColumn("Date");
                        ImGui::TableSetupColumn("Type");

                        ImGui::TableHeadersRow();

                        for (auto &t : state.searchTransactions)
                        {
                            ImVec4 amtColor = (t.type == Transaction::TransactionType::Expense) ? ImVec4(1.0f, 0.0f, 0.0f, 1.0f) : ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("%s", t.category.c_str());

                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text("%s", t.label.c_str());

                            ImGui::TableSetColumnIndex(2);
                            ImGui::TextColored(amtColor, "%.2f", t.amount);

                            ImGui::TableSetColumnIndex(3);
                            ImGui::Text("%s", Transaction::toString(t.method).c_str());

                            ImGui::TableSetColumnIndex(4);
                            ImGui::Text("%s", t.date.c_str());

                            ImGui::TableSetColumnIndex(5);
                            ImGui::Text("%s", Transaction::toString(t.type).c_str());
                        }

                        ImGui::EndTable();
                    }
                }

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();

            break;
        }

        case Page::Analytics:
        {

            // TODO:  DONE
            //       CARDS: {Show total income, total expenses, net savings for selected period}
            //       DONE

            // state.forceReloadAllData();
            loadAnalyticsData(state);

            if (!state.monthlySpendings.empty()) // Use better condition
            {
                static std::vector<std::string> catVec(state.categories.begin(), state.categories.end());
                static std::unordered_map<std::string, std::vector<double>> catAmounts;

                for (auto &cat : state.categories)
                {
                    std::vector<double> vals;
                    for (auto &month : state.months)
                    {
                        double amt = 0;
                        if (state.monthlySpendings[month].count(cat))
                            amt = state.monthlySpendings[month][cat];
                        vals.push_back(amt);
                    }
                    catAmounts[cat] = vals;
                }

                ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(30, 30));

                if (ImPlot::BeginPlot("Expenses by Category", ImVec2(600, 400), ImPlotFlags_NoFrame | ImPlotFlags_NoLegend))
                {

                    std::vector<const char *> labels;
                    std::vector<double> tickPos;
                    int idx = 0;
                    for (int i = 0; i < catVec.size(); i++)
                    {
                        labels.push_back(catVec[i].c_str());
                        tickPos.push_back(i);
                    }

                    ImPlot::SetupAxes("Category", "INR", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    ImPlot::SetupAxisTicks(ImAxis_X1, tickPos.data(), tickPos.size(), labels.data());

                    idx = 0;
                    for (auto &[cat, amtVec] : catAmounts)
                    {
                        double total = std::accumulate(amtVec.begin(), amtVec.end(), 0.0);
                        ImPlot::PushStyleColor(ImPlotCol_Fill, ImPlot::GetColormapColor(idx));
                        ImPlot::PlotBars("##cat", &total, 1, 0.3, idx);
                        ImPlot::PopStyleColor();
                        idx++;
                    }

                    ImPlot::EndPlot();
                }

                ImPlot::PopStyleVar();

                ImGui::SameLine();

                ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(30, 30));

                if (ImPlot::BeginPlot("Spending by Categories by Months", ImVec2(600, 400), ImPlotFlags_NoFrame))
                {
                    const char *monthsByName[12] = {"Jan", "Feb", "Mar", "April", "May", "Jun",
                                                    "July", "Aug", "Sep", "Oct", "Nov", "Dec"};
                    ImPlot::SetupAxes("Month", "Amount", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_Opposite);

                    // Safety check for months
                    if (!state.months.empty())
                    {
                        std::vector<double> monthTicks;
                        std::vector<const char *> monthLabels;

                        for (size_t i = 0; i < state.months.size(); i++)
                        {
                            monthTicks.push_back(static_cast<double>(i));
                        }

                        for (auto &m : state.monthlySpendings)
                        {
                            int mCode = m.first;
                            int monthIdx = (mCode / 10000) - 1;
                            monthLabels.push_back(monthsByName[monthIdx]);
                            // std::cout << mCode << std::endl;
                        }

                        ImPlot::SetupAxisTicks(ImAxis_X1, monthTicks.data(), static_cast<int>(monthTicks.size()), monthLabels.data());

                        std::vector<double> monthsX(state.months.size());
                        for (size_t i = 0; i < state.months.size(); ++i)
                            monthsX[i] = static_cast<double>(i);

                        for (auto &[cat, amt] : catAmounts)
                        {
                            ImPlot::PlotLine(cat.c_str(), monthsX.data(), amt.data(), static_cast<int>(state.months.size()));
                        }
                    }

                    ImPlot::EndPlot();
                    ImPlot::PopStyleVar();
                }

                else
                {
                    ImGui::PushFont(poppinsMediumItalic);
                    ImGui::TextColored(ImVec4(1.0f, 0.522f, 0.522f, 1.0f), "%s", "No expenses recorded.");
                    ImGui::PopFont();
                }
                // DRAWING THE HEATMAP HERE. MIGHT MISS IT--------------------------------
                ImGui::SameLine();
                DrawHeatMap(state);
            }
            else
            {
                ImGui::PushFont(poppinsMediumItalic);
                ImGui::TextColored(ImVec4(1.0f, 0.522f, 0.522f, 1.0f), "%s", "No data available for analytics");
                ImGui::PopFont();
            }

            break;
        }
        }

        ImGui::End(); // Home window

        // --- RENDER ---
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}