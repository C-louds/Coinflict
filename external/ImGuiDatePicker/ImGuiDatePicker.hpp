#pragma once
#include <imgui.h>
#include <string>
#include <vector>

#ifndef IMGUI_DATEPICKER_YEAR_MIN
#define IMGUI_DATEPICKER_YEAR_MIN 1900
#endif // !IMGUI_DATEPICKER_YEAR_MIN

#ifndef IMGUI_DATEPICKER_YEAR_MAX
#define IMGUI_DATEPICKER_YEAR_MAX 3000
#endif // !IMGUI_DATEPICKER_YEAR_MAX

namespace ImGui
{
    IMGUI_API bool DatePickerEx(const std::string &label, tm &v, ImFont *altFont, bool clampToBorder = false, float itemSpacing = 130.0f);

    IMGUI_API bool DatePicker(const std::string &label, tm &v, bool clampToBorder = false, float itemSpacing = 130.0f);

    IMGUI_API std::vector<int> CalendarWeek(int week, int startDay, int daysInMonth);
    IMGUI_API int DayOfWeek(int dayOfMonth, int month, int year) noexcept;
    IMGUI_API constexpr static bool IsLeapYear(int year) noexcept;
    IMGUI_API int NumDaysInMonth(int month, int year); 
    IMGUI_API int NumWeeksInMonth(int month, int year);

}
