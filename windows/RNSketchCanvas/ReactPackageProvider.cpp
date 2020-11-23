#include "pch.h"
#include "ReactPackageProvider.h"
#if __has_include("ReactPackageProvider.g.cpp")
#  include "ReactPackageProvider.g.cpp"
#endif

#include "RNSketchCanvasViewManager.h"

using namespace winrt::Microsoft::ReactNative;

namespace winrt::RNSketchCanvas::implementation {
  void ReactPackageProvider::CreatePackage(IReactPackageBuilder const &packageBuilder) noexcept {
    packageBuilder.AddViewManager(L"RNSketchCanvasViewManager", []() { return winrt::make<RNSketchCanvasViewManager>(); });
  }
}
