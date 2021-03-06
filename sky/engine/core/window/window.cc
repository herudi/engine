// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/core/window/window.h"

#include "sky/engine/core/compositing/Scene.h"
#include "sky/engine/core/script/ui_dart_state.h"
#include "sky/engine/tonic/dart_converter.h"
#include "sky/engine/tonic/dart_invoke.h"
#include "sky/engine/tonic/dart_library_natives.h"

namespace blink {
namespace {

void ScheduleFrame(Dart_NativeArguments args) {
  UIDartState::Current()->window()->client()->ScheduleFrame();
}

void Render(Dart_NativeArguments args) {
  Dart_Handle exception = nullptr;
  Scene* scene = DartConverter<Scene*>::FromArguments(args, 1, exception);
  if (exception) {
    Dart_ThrowException(exception);
    return;
  }
  UIDartState::Current()->window()->client()->Render(scene);
}


void FlushRealTimeEvents(Dart_NativeArguments args) {
  UIDartState::Current()->window()->client()->FlushRealTimeEvents();
}

}  // namespace

WindowClient::~WindowClient() {
}

Window::Window(WindowClient* client)
  : client_(client) {
}

Window::~Window() {
}

void Window::DidCreateIsolate() {
  library_.Set(DartState::Current(), Dart_LookupLibrary(ToDart("dart:ui")));
}

void Window::UpdateWindowMetrics(const sky::ViewportMetricsPtr& metrics) {
  DartState* dart_state = library_.dart_state().get();
  if (!dart_state)
    return;
  DartState::Scope scope(dart_state);
  double device_pixel_ratio = metrics->device_pixel_ratio;
  DartInvokeField(library_.value(), "_updateWindowMetrics", {
    ToDart(device_pixel_ratio),
    ToDart(metrics->physical_width / device_pixel_ratio),
    ToDart(metrics->physical_height / device_pixel_ratio),
    ToDart(metrics->physical_padding_top / device_pixel_ratio),
    ToDart(metrics->physical_padding_right / device_pixel_ratio),
    ToDart(metrics->physical_padding_bottom / device_pixel_ratio),
    ToDart(metrics->physical_padding_left / device_pixel_ratio),
  });
}

void Window::UpdateLocale(const std::string& language_code,
                          const std::string& country_code) {
  DartState* dart_state = library_.dart_state().get();
  if (!dart_state)
    return;
  DartState::Scope scope(dart_state);

  DartInvokeField(library_.value(), "_updateLocale", {
    StdStringToDart(language_code),
    StdStringToDart(country_code),
  });
}

void Window::PushRoute(const std::string& route) {
  DartState* dart_state = library_.dart_state().get();
  if (!dart_state)
    return;
  DartState::Scope scope(dart_state);

  DartInvokeField(library_.value(), "_pushRoute", {
    StdStringToDart(route),
  });
}

void Window::PopRoute() {
  DartState* dart_state = library_.dart_state().get();
  if (!dart_state)
    return;
  DartState::Scope scope(dart_state);

  DartInvokeField(library_.value(), "_popRoute", {});
}

void Window::DispatchPointerPacket(const pointer::PointerPacketPtr& packet) {
  DartState* dart_state = library_.dart_state().get();
  if (!dart_state)
    return;
  DartState::Scope scope(dart_state);

  Dart_Handle data_handle = Dart_NewTypedData(Dart_TypedData_kByteData,
                                              packet->GetSerializedSize());
  if (Dart_IsError(data_handle))
    return;

  Dart_TypedData_Type type;
  void* data;
  intptr_t len;
  if (Dart_IsError(Dart_TypedDataAcquireData(data_handle, &type, &data, &len)))
    return;

  packet->Serialize(data, len);

  Dart_TypedDataReleaseData(data_handle);

  DartInvokeField(library_.value(), "_dispatchPointerPacket", {data_handle});
}

void Window::BeginFrame(base::TimeTicks frameTime) {
  DartState* dart_state = library_.dart_state().get();
  if (!dart_state)
    return;
  DartState::Scope scope(dart_state);

  int64_t microseconds = (frameTime - base::TimeTicks()).InMicroseconds();

  DartInvokeField(library_.value(), "_beginFrame", {
    Dart_NewInteger(microseconds),
  });
}

void Window::OnAppLifecycleStateChanged(sky::AppLifecycleState state) {
  DartState* dart_state = library_.dart_state().get();
  if (!dart_state)
    return;
  DartState::Scope scope(dart_state);

  DartInvokeField(library_.value(), "_onAppLifecycleStateChanged", {
    ToDart(static_cast<int>(state))
  });
}

void Window::RegisterNatives(DartLibraryNatives* natives) {
  natives->Register({
    { "Window_scheduleFrame", ScheduleFrame, 1, true },
    { "Window_render", Render, 2, true },
    { "Scheduler_FlushRealTimeEvents", FlushRealTimeEvents, 1, true},
  });
}

}  // namespace blink
