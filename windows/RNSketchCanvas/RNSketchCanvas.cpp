#include "pch.h"
#include "JSValueXaml.h"
#include "RNSketchCanvas.h"
#include "Utility.h"
#include "RNSketchCanvasModule.g.cpp"

namespace winrt
{
  using namespace Microsoft::ReactNative;
  using namespace Windows::Data::Json;
  using namespace Windows::Foundation;
  using namespace Windows::UI;
  using namespace Windows::UI::Popups;
  using namespace Windows::UI::Xaml;
  using namespace Windows::UI::Xaml::Controls;
  using namespace Windows::UI::Xaml::Input;
  using namespace Windows::UI::Xaml::Media;
  using namespace Windows::Foundation::Numerics;
  using namespace Microsoft::Graphics::Canvas;
  using namespace Microsoft::Graphics::Canvas::UI::Xaml;
} // namespace winrt

namespace winrt::RNSketchCanvas::implementation
{

  RNSketchCanvasModule::RNSketchCanvasModule(winrt::IReactContext const& reactContext) : m_reactContext(reactContext)
  {
    // Sets a Transparent background so that it receives mouse events in the JavaScript side.
    mCanvasControl = Microsoft::Graphics::Canvas::UI::Xaml::CanvasControl();
    this->Children().Append(mCanvasControl);
    mCanvasControl.Background(SolidColorBrush(Colors::Transparent()));
    // TODO: event tokens from these. Or use the revokers.
    mCanvasControl.Draw({ this, &RNSketchCanvasModule::OnCanvasDraw });
    mCanvasControl.SizeChanged({ this, &RNSketchCanvasModule::OnCanvasSizeChanged });

    // TODO: hook up events from the controll
    /*m_textChangedRevoker = this->TextChanged(winrt::auto_revoke,
        [ref = get_weak()](auto const& sender, auto const& args) {
        if (auto self = ref.get()) {
            self->OnTextChanged(sender, args);
        }
    });*/
  }

  void RNSketchCanvasModule::OnTextChanged(winrt::Windows::Foundation::IInspectable const&,
    winrt::Windows::UI::Xaml::Controls::TextChangedEventArgs const&)
  {
    // TODO: example sending event on text changed
   /* auto text = this->Text();
    m_reactContext.DispatchEvent(
      *this,
      L"sampleEvent",
      [&](winrt::Microsoft::ReactNative::IJSValueWriter const& eventDataWriter) noexcept {
        eventDataWriter.WriteObjectBegin();
        WriteProperty(eventDataWriter, L"text", text);
        eventDataWriter.WriteObjectEnd();
      }
    );*/
  }

  winrt::Windows::Foundation::Collections::
    IMapView<winrt::hstring, winrt::Microsoft::ReactNative::ViewManagerPropertyType>
    RNSketchCanvasModule::NativeProps() noexcept
  {
    // TODO: define props here
    auto nativeProps = winrt::single_threaded_map<hstring, ViewManagerPropertyType>();
    nativeProps.Insert(L"sampleProp", ViewManagerPropertyType::String);
    return nativeProps.GetView();
  }

  void RNSketchCanvasModule::UpdateProperties(winrt::Microsoft::ReactNative::IJSValueReader const& propertyMapReader) noexcept
  {
    // TODO: handle the props here
    /*const JSValueObject &propertyMap = JSValue::ReadObjectFrom(propertyMapReader);
    for (auto const &pair : propertyMap) {
        auto const &propertyName = pair.first;
        auto const &propertyValue = pair.second;
        if (propertyName == "sampleProp") {
            if (propertyValue != nullptr) {
                auto const &value = propertyValue.AsString();
                this->Text(winrt::to_hstring(value));
            } else {
                this->Text(L"");
            }
        }
    }*/
  }

  winrt::Microsoft::ReactNative::ConstantProviderDelegate RNSketchCanvasModule::ExportedCustomBubblingEventTypeConstants() noexcept
  {
    return nullptr;
  }

  winrt::Microsoft::ReactNative::ConstantProviderDelegate RNSketchCanvasModule::ExportedCustomDirectEventTypeConstants() noexcept
  {
    return [](winrt::IJSValueWriter const& constantWriter)
    {
      // TODO: define events emitted by the control
      WriteCustomDirectEventTypeConstant(constantWriter, "sampleEvent");
    };
  }

  winrt::Windows::Foundation::Collections::IVectorView<winrt::hstring> RNSketchCanvasModule::Commands() noexcept
  {
    auto commands = winrt::single_threaded_vector<hstring>();
    commands.Append(L"addPoint");
    commands.Append(L"newPath");
    commands.Append(L"clear");
    commands.Append(L"addPath");
    commands.Append(L"deletePath");
    commands.Append(L"save");
    commands.Append(L"endPath");
    return commands.GetView();
  }

  void RNSketchCanvasModule::DispatchCommand(winrt::hstring const& commandId, winrt::Microsoft::ReactNative::IJSValueReader const& commandArgsReader) noexcept
  {
    // TODO: handle commands here
    auto commandArgs = JSValue::ReadArrayFrom(commandArgsReader);
    if (commandId == L"addPoint")
    {
      this->addPoint(commandArgs[0].AsSingle(), commandArgs[1].AsSingle());
    } else if (commandId == L"newPath")
    {
      this->newPath(commandArgs[0].AsInt32(), commandArgs[1].AsUInt32(), commandArgs[2].AsSingle());
    } else if (commandId == L"clear")
    {
      this->clear();
    } else if (commandId == L"addPath")
    {
      const auto& path = commandArgs[3].AsArray();
      std::vector<float2> pointPath;
      for (int i = 0; i < path.size(); i++)
      {
        std::string pointstring = path[i].AsString();
        int commaIndex = pointstring.find(",");
        pointPath.push_back(
          float2(
            std::stof(pointstring.substr(0, commaIndex)),
            std::stof(pointstring.substr(commaIndex + 1, std::string::npos))
          )
        );
      }
      this->addPath(commandArgs[0].AsInt32(), commandArgs[1].AsUInt32(), commandArgs[2].AsSingle(), pointPath);
    } else if (commandId == L"deletePath")
    {
      this->deletePath(commandArgs[0].AsInt32());
    } else if (commandId == L"endPath")
    {
      this->end();
    }
  }
  void RNSketchCanvasModule::clear()
  {
    for (SketchData* data : mPaths)
    {
      delete data;
    }
    mPaths.clear();
    mCurrentPath = nullptr;
    mNeedsFullRedraw = true;
    mCanvasControl.Invalidate();
  }
  void RNSketchCanvasModule::newPath(int32_t id, uint32_t strokeColor, float strokeWidth)
  {
    Color color = Utility::uint32ToColor(strokeColor);
    mCurrentPath = new SketchData(id, color, strokeWidth);
    mPaths.push_back(mCurrentPath);
    //is Erase, do we need to disable hardware acceleration? through ForceSoftwareRenderer
    mCanvasControl.Invalidate();
  }

  void RNSketchCanvasModule::addPoint(float x, float y)
  {
    Rect updateRect = mCurrentPath->addPoint(Point(x, y));
    if (mCurrentPath->isTranslucent)
    {
      auto session = mTranslucentDrawingCanvas.value().CreateDrawingSession();
      session.Clear(Colors::Transparent());
      mCurrentPath->draw(session);
    } else
    {
      auto session = mDrawingCanvas.value().CreateDrawingSession();
      mCurrentPath->drawLastPoint(session);
    }
    mCanvasControl.Invalidate();
  }
  void RNSketchCanvasModule::addPath(int32_t id, uint32_t strokeColor, float strokeWidth, std::vector<float2> points)
  {
    bool exist = false;
    for (SketchData* data : mPaths)
    {
      if (data->id == id)
      {
        return;
      }
    }
    if (!exist)
    {
      SketchData* newPath = new SketchData(id, Utility::uint32ToColor(strokeColor), strokeWidth, points);
      mPaths.push_back(newPath);
      {
        auto session = mDrawingCanvas.value().CreateDrawingSession();
        newPath->draw(session);
      }
      mCanvasControl.Invalidate();
    }
  }
  void RNSketchCanvasModule::deletePath(int32_t id)
  {
    int index = -1;
    for (int i = 0; i < mPaths.size(); i++)
    {
      if (mPaths[i]->id == id)
      {
        index = i;
        break;
      }
    }
    if (index > -1)
    {
      SketchData* path = mPaths[index];
      mPaths.erase(mPaths.begin() + index);
      delete path;
      mNeedsFullRedraw = true;
      mCanvasControl.Invalidate();
    }
  }
  void RNSketchCanvasModule::end()
  {
    if (mCurrentPath != nullptr)
    {
      if (mCurrentPath->isTranslucent)
      {
        {
          auto session = mDrawingCanvas.value().CreateDrawingSession();
          mCurrentPath->draw(session);
        }
        {
          auto session = mTranslucentDrawingCanvas.value().CreateDrawingSession();
          session.Clear(Colors::Transparent());
        }
      }
      mCurrentPath = nullptr;
    }
  }
  void RNSketchCanvasModule::OnCanvasDraw(CanvasControl const& canvas, CanvasDrawEventArgs const& args)
  {
    if (mNeedsFullRedraw && mDrawingCanvas.has_value())
    {
      auto session = mDrawingCanvas.value().CreateDrawingSession();
      session.Clear(Colors::Transparent());
      for (SketchData* path : mPaths)
      {
        path->draw(session);
      }
      mNeedsFullRedraw = false;
    }

    if (mDrawingCanvas.has_value())
    {
      args.DrawingSession().DrawImage(mDrawingCanvas.value());
    }
    if (mTranslucentDrawingCanvas.has_value() && mCurrentPath != nullptr && mCurrentPath->isTranslucent)
    {
      args.DrawingSession().DrawImage(mTranslucentDrawingCanvas.value());
    }

  }
  void RNSketchCanvasModule::OnCanvasSizeChanged(const IInspectable canvas, Windows::UI::Xaml::SizeChangedEventArgs const& args)
  {
    Size newSize = args.NewSize();
    if (newSize.Width >= 0 && newSize.Height >= 0)
    {
      mDrawingCanvas = CanvasRenderTarget(CanvasDevice::GetSharedDevice(), newSize.Width, newSize.Height, mCanvasControl.Dpi());
      {
        auto session = mDrawingCanvas.value().CreateDrawingSession();
        session.Clear(Colors::Transparent());
      }
      mTranslucentDrawingCanvas = CanvasRenderTarget(CanvasDevice::GetSharedDevice(), newSize.Width, newSize.Height, mCanvasControl.Dpi());
      {
        auto session = mTranslucentDrawingCanvas.value().CreateDrawingSession();
        session.Clear(Colors::Transparent());
      }
      mNeedsFullRedraw = true;
      mCanvasControl.Invalidate();
    }
  }

}