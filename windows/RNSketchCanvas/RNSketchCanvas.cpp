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
} // namespace winrt

namespace winrt::RNSketchCanvas::implementation
{

  RNSketchCanvasModule::RNSketchCanvasModule(winrt::IReactContext const& reactContext) : m_reactContext(reactContext)
  {
    // Sets a Transparent background so that it receives mouse events in the JavaScript side.
    this->Background(SolidColorBrush(Colors::Transparent()));
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
      this->newPath(commandArgs[0].AsInt32(),commandArgs[1].AsUInt32(), commandArgs[2].AsSingle());
    } else if (commandId == L"clear")
    {
      this->clear();
    } else if (commandId == L"addPath")
    {
      const auto& path = commandArgs[3].AsArray();
      std::vector<Point> pointPath;
      for (int i = 0; i < path.size(); i++)
      {
        std::string pointstring = path[i].AsString();
        int commaIndex = pointstring.find(",");
        pointPath.push_back(
          Point(
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
    this->Children().Clear();
  }
  void RNSketchCanvasModule::newPath(int32_t id, uint32_t strokeColor, float strokeWidth)
  {
    mCurrentPath = new SketchData(*this, id, Utility::uint32ToColor(strokeColor), strokeWidth);
    mPaths.push_back(mCurrentPath);
    //is Erase
  }

  void RNSketchCanvasModule::addPoint(float x, float y)
  {
    Rect updateRect = mCurrentPath->addPoint(Point(x, y));
    //isTranslucent
    mCurrentPath->drawLastPoint(*this);
  }
  void RNSketchCanvasModule::addPath(int32_t id, uint32_t strokeColor, float strokeWidth, std::vector<winrt::Windows::Foundation::Point> points)
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
      SketchData* newPath = new SketchData(*this, id, Utility::uint32ToColor(strokeColor), strokeWidth, points);
      mPaths.push_back(newPath);
      //isErase?
      newPath->draw(*this);
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
      path->removeFromCanvas(*this);
      delete path;
    }
  }
  void RNSketchCanvasModule::end()
  {
    if (mCurrentPath != nullptr)
    {
      //translucent
      mCurrentPath->draw(*this);
      mCurrentPath = nullptr;
    }
  }
}