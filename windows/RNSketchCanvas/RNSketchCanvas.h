#pragma once

#include "pch.h"
#include "winrt/Microsoft.ReactNative.h"
#include "NativeModules.h"
#include "RNSketchCanvasView.g.h"
#include "SketchData.h"

namespace winrt::RNSketchCanvas::implementation
{

  class RNSketchCanvasView : public RNSketchCanvasViewT<RNSketchCanvasView>
  {
  public:
    RNSketchCanvasView(Microsoft::ReactNative::IReactContext const& reactContext);

    void openImageFile(std::string filename, std::string directory, std::string mode);

    static winrt::Windows::Foundation::Collections::
      IMapView<winrt::hstring, winrt::Microsoft::ReactNative::ViewManagerPropertyType>
      NativeProps() noexcept;
    void UpdateProperties(winrt::Microsoft::ReactNative::IJSValueReader const& propertyMapReader) noexcept;

    
    static winrt::Microsoft::ReactNative::ConstantProviderDelegate
      ExportedViewConstants() noexcept;

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
    void addPath(int32_t id, uint32_t strokeColor, float strokeWidth, std::vector<winrt::Windows::Foundation::Numerics::float2> points);
    void deletePath(int32_t id);
    void end();
    void save(std::string format, std::string folder, std::string filename, bool transparent, bool includeImage, bool includeText, bool cropToImageSize);

    IAsyncOperation<winrt::hstring> getBase64(std::string format, bool transparent, bool includeImage, bool includeText, bool cropToImageSize);

  private:
    std::vector<SketchData*> mPaths;
    SketchData* mCurrentPath = nullptr;
    Microsoft::Graphics::Canvas::UI::Xaml::CanvasControl mCanvasControl;
    bool mNeedsFullRedraw = true;
    std::optional<winrt::Microsoft::Graphics::Canvas::CanvasRenderTarget> mDrawingCanvas = std::nullopt;
    std::optional<winrt::Microsoft::Graphics::Canvas::CanvasRenderTarget> mTranslucentDrawingCanvas = std::nullopt;
    winrt::Windows::Foundation::Numerics::float2 mOldSize = winrt::Windows::Foundation::Numerics::float2(-1.f, -1.f);

    std::optional<winrt::Microsoft::Graphics::Canvas::CanvasBitmap> mBackgroundImage;
    int mOriginalWidth, mOriginalHeight;
    std::string mContentMode;

    IAsyncOperation<winrt::hstring> saveHelper(std::string format, std::string folder, std::string filename, bool transparent, bool includeImage, bool includeText, bool cropToImageSize);

    void OnTextChanged(winrt::Windows::Foundation::IInspectable const& sender,
      winrt::Windows::UI::Xaml::Controls::TextChangedEventArgs const& args);
    winrt::Windows::UI::Xaml::Controls::TextBox::TextChanged_revoker m_textChangedRevoker{};

    Microsoft::ReactNative::IReactContext m_reactContext{ nullptr };
    void OnCanvasDraw(Microsoft::Graphics::Canvas::UI::Xaml::CanvasControl const&, Microsoft::Graphics::Canvas::UI::Xaml::CanvasDrawEventArgs const&);
    void OnCanvasSizeChanged(const winrt::Windows::Foundation::IInspectable, Windows::UI::Xaml::SizeChangedEventArgs const&);
    Microsoft::Graphics::Canvas::UI::Xaml::CanvasControl::Draw_revoker mCanvasDrawRevoker{};
    Microsoft::Graphics::Canvas::UI::Xaml::CanvasControl::SizeChanged_revoker mCanvaSizeChangedRevoker{};

    Microsoft::Graphics::Canvas::CanvasBitmap createImage(bool transparent, bool includeImage, bool includeText, bool cropToImageSize);
  };
}

namespace winrt::RNSketchCanvas::factory_implementation
{
  struct RNSketchCanvasView : RNSketchCanvasViewT<RNSketchCanvasView, implementation::RNSketchCanvasView> {};
}
