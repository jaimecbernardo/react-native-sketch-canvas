#pragma once
#include <vector>

namespace winrt::RNSketchCanvas::implementation
{
  class SketchData
  {
  public:
    std::vector<winrt::Windows::Foundation::Point> points;
    int id;
    winrt::Windows::UI::Color strokeColor;
    float strokeWidth;
    bool isTranslucent;

    SketchData(winrt::Windows::UI::Xaml::Controls::Canvas, int id, winrt::Windows::UI::Color strokeColor, float strokeWidth);
    SketchData(winrt::Windows::UI::Xaml::Controls::Canvas, int id, winrt::Windows::UI::Color strokeColor, float strokeWidth, std::vector<winrt::Windows::Foundation::Point> points);
    
    static winrt::Windows::Foundation::Point midPoint(const winrt::Windows::Foundation::Point&, const winrt::Windows::Foundation::Point&);

    winrt::Windows::Foundation::Rect addPoint(winrt::Windows::Foundation::Point& p);

    void drawLastPoint(winrt::Windows::UI::Xaml::Controls::Canvas);
    void draw(winrt::Windows::UI::Xaml::Controls::Canvas);
    void draw(winrt::Windows::UI::Xaml::Controls::Canvas, int);
    void removeFromCanvas(winrt::Windows::UI::Xaml::Controls::Canvas);

  private:
    std::optional<winrt::Windows::UI::Xaml::Shapes::Path> mPath;
    std::optional<winrt::Windows::Foundation::Rect> mDirty;

    winrt::Windows::UI::Xaml::Shapes::Path evaluatePath();
    void addPointToPath(
      winrt::Windows::UI::Xaml::Shapes::Path& path,
      const winrt::Windows::Foundation::Point& tPoint,
      const winrt::Windows::Foundation::Point& pPoint,
      const winrt::Windows::Foundation::Point& point
    );

  };
}
