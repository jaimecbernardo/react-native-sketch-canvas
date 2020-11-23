#pragma once
namespace winrt::RNSketchCanvas::implementation
{
  class Utility
  {
  public:
    static bool isSameColor(winrt::Windows::UI::Color, winrt::Windows::UI::Color) noexcept;
    static winrt::Windows::UI::Color uint32ToColor(uint32_t) noexcept;
  };
}
