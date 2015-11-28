#pragma once

#include "Game.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Popups;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Platform;

ref class App sealed : IFrameworkView
{
private:
	bool WindowClosed;
	Game game;

public:
	virtual void Initialize(CoreApplicationView^ appView);
	virtual void Uninitialize();
	virtual void SetWindow(CoreWindow^ window);
	virtual void Load(String^ entryPoint);
	virtual void Run();

	// App event
	virtual void OnActivated(CoreApplicationView^ coreAppView, IActivatedEventArgs^ args);
	virtual void OnSuspending(Object^ sender, SuspendingEventArgs^ args);
	virtual void OnResuming(Object^ sender, SuspendingEventArgs^ args);

	// Windows event
	virtual void PointerPressed(CoreWindow^ window, PointerEventArgs^ args);
	virtual void OnKeyDown(Windows::UI::Core::CoreWindow ^sender, Windows::UI::Core::KeyEventArgs ^args);
	void OnClosed(Windows::UI::Core::CoreWindow ^sender, Windows::UI::Core::CoreWindowEventArgs ^args);
};

ref class AppSource sealed : IFrameworkViewSource
{
public:
	virtual IFrameworkView^ CreateView()
	{
		return ref new App();
	}
};

int main(Array<String^>^ args)
{
	CoreApplication::Run(ref new AppSource());
	
	return 0;
}
