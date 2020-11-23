#include "pch.h"
#include "Utility.h"

namespace winrt::RNSketchCanvas::implementation
{
  bool Utility::isSameColor(winrt::Windows::UI::Color color1, winrt::Windows::UI::Color color2) noexcept
  {
    if (color1.R == color2.R && color1.G == color2.G && color1.B == color2.B && color1.A == color2.A)
    {
      return true;
    }
    return false;
  }

  winrt::Windows::UI::Color Utility::uint32ToColor(uint32_t color) noexcept
  {
    winrt::Windows::UI::Color result;
    result.A = (color >> 24) & 0xff;
    result.R = (color >> 16) & 0xff;
    result.G = (color >> 8) & 0xff;
    result.B = color & 0xff;
    return result;
  }

}
