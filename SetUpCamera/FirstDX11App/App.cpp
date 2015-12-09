#include "pch.h"
#include "App.h"


void App::Initialize(CoreApplicationView^ appView)
{
	appView->Activated +=
		ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);
	CoreApplication::Suspending += 
		ref new EventHandler<SuspendingEventArgs^>(this, &App::OnSuspending);

	WindowClosed = false;
}

void App::Run()
{
	CoreWindow^ window = CoreWindow::GetForCurrentThread();

	// Init game
	game.Initialize();

	while (!WindowClosed)
	{
		window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

		// Game code
		game.Update();
		game.Render();
	}
}

void App::Uninitialize()
{
	
}

void App::SetWindow(CoreWindow^ window)
{
	window->PointerPressed += 
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::PointerPressed);
	window->KeyDown += 
		ref new TypedEventHandler<CoreWindow ^, KeyEventArgs ^>(this, &App::OnKeyDown);
	window->Closed += 
		ref new TypedEventHandler<CoreWindow ^, CoreWindowEventArgs ^>(this, &App::OnClosed);
}

void App::Load(String ^ entryPoint)
{
	
}

void App::PointerPressed(CoreWindow^ window, PointerEventArgs^ args)
{
	MessageDialog dialog("Thank you for noticing this!", "Notice!");
	dialog.ShowAsync();
}

void App::OnKeyDown(CoreWindow^ sender, KeyEventArgs^ args)
{
	if (args->VirtualKey == VirtualKey::A)
	{

	}
}

void App::OnActivated(CoreApplicationView^ coreAppView, IActivatedEventArgs^ args)
{
	CoreWindow^ window = CoreWindow::GetForCurrentThread();
	window->Activate();
}

void App::OnSuspending(Object^ sender, SuspendingEventArgs^ args)
{

}

void App::OnResuming(Object^ sender, SuspendingEventArgs^ args)
{

}

void App::OnClosed(CoreWindow ^sender, CoreWindowEventArgs ^args)
{
	WindowClosed = true;
}
