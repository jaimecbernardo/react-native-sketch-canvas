#pragma once

#include "pch.h"
#include "winrt/Microsoft.ReactNative.h"
#include "NativeModules.h"
#include "RNSketchCanvasModule.g.h"
#include "SketchData.h"

namespace winrt::RNSketchCanvas::implementation
{

  class RNSketchCanvasModule : public RNSketchCanvasModuleT<RNSketchCanvasModule>
  {
  public:
    RNSketchCanvasModule(Microsoft::ReactNative::IReactContext const& reactContext);

    static winrt::Windows::Foundation::Collections::
      IMapView<winrt::hstring, winrt::Microsoft::ReactNative::ViewManagerPropertyType>
      NativeProps() noexcept;
    void UpdateProperties(winrt::Microsoft::ReactNative::IJSValueReader const& propertyMapReader) noexcept;


    static winrt::Microsoft::ReactNative::ConstantProviderDelegate
      ExportedCustomBubblingEventTypeConstants() noexcept;
    static winrt::Microsoft::ReactNative::ConstantProviderDelegate
      ExportedCustomDirectEventTypeConstants() noexcept;

    static winrt::Windows::Foundation::Collections::IVectorView<winrt::hstring> Commands() noexcept;
    void DispatchCommand(
      winrt::hstring const& commandId,
      winrt::Microsoft::ReactNative::IJSValueReader const& commandArgsReader) noexcept;

    void clear();
    void newPath(int32_t id, uint32_t strokeColor, float strokeWidth);
    void addPoint(float x, float y);
    void addPath(int32_t id, uint32_t strokeColor, float strokeWidth, std::vector<winrt::Windows::Foundation::Point> points);
    void deletePath(int32_t id);
    void end();

  private:
    std::vector<SketchData*> mPaths;
    SketchData* mCurrentPath = nullptr;

    void OnTextChanged(winrt::Windows::Foundation::IInspectable const& sender,
      winrt::Windows::UI::Xaml::Controls::TextChangedEventArgs const& args);
    Microsoft::ReactNative::IReactContext m_reactContext{ nullptr };
    winrt::Windows::UI::Xaml::Controls::TextBox::TextChanged_revoker m_textChangedRevoker{};
  };
}

namespace winrt::RNSketchCanvas::factory_implementation
{
  struct RNSketchCanvasModule : RNSketchCanvasModuleT<RNSketchCanvasModule, implementation::RNSketchCanvasModule> {};
}
