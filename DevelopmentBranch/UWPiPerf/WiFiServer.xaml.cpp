//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

//
// WiFiServer.xaml.cpp
// Implementation of the WiFiServer class
//

#include "pch.h"
#include "WiFiServer.xaml.h"

#include "socket_threads.h"

using namespace SDKTemplate;
using namespace SDKTemplate::WiFiTestTools;

using namespace Concurrency;
using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Networking;
using namespace Windows::Networking::Connectivity;
using namespace Windows::Networking::Sockets;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;

WiFiServer::WiFiServer()
{
    InitializeComponent();
    this->localHostItems = ref new Vector<LocalHostItem^>();
	mThreadHandle = INVALID_HANDLE_VALUE;
	mIperfIsRun = false;

	StartListener->IsEnabled = true;
	StopListener->IsEnabled = false;
}

/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>
void WiFiServer::OnNavigatedTo(NavigationEventArgs^ e)
{
    // A pointer back to the main page.  This is needed if you want to call methods in MainPage such
    // as NotifyUser()
    rootPage = MainPage::Current;
	MainPage::ChildFrame = this;
	MainPage::ChildType = FrameType::WiFiServer;

}

void WiFiServer::StartListener_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    // Overriding the listener here is safe as it will be deleted once all references to it are gone. However,
    // in many cases this is a dangerous pattern to override data semi-randomly (each time user clicked the button)
    // so we block it here.
	int paramc = 0;

	// for port
	char port_param[15] = { 0 };
	char port[10] = { 0 };

	//for time interval
	char interval[10] = { 0 };
	char interval_param[15];

	char * params[20];

	params[paramc] = "-s";
	paramc++;

	// get port
	if (ServerPortForListener->Text != nullptr && ServerPortForListener->Text->Length() < 10)
	{
		WideCharToMultiByte(CP_ACP, // ANSI Code Page
			0, // No special handling of unmapped chars
			ServerPortForListener->Text->Data(), // wide-character string to be converted
			ServerPortForListener->Text->Length(),
			port,
			9,
			"5001", NULL); // Unrepresented char replacement - Use Default
		sprintf(port_param, "-p %s", port);
		params[paramc] = port_param;
		paramc++;
	}

	// get interval
	if (TimeInterval->Text != nullptr && TimeInterval->Text->Length() < 10)
	{
		WideCharToMultiByte(CP_ACP, // ANSI Code Page
			0, // No special handling of unmapped chars
			TimeInterval->Text->Data(), // wide-character string to be converted
			TimeInterval->Text->Length(),
			interval,
			9,
			"2", NULL); // Unrepresented char replacement - Use Default
		sprintf(interval_param, "-i %s", interval);
		params[paramc] = interval_param;
		paramc++;
	}

	mThreadHandle = StartIPerfThread(paramc, params);

	ReportMessageBox->Text = L"";

	mIperfIsRun = true;

	StartListener->IsEnabled = false;
	StopListener->IsEnabled = true;
	
	// After start socket server thread, disable 'Connect to Listener' item
	rootPage->ScenarioItemEnable(1, false);
}

void SDKTemplate::WiFiTestTools::WiFiServer::StopListener_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	EndIPerfThread(true);

	mIperfIsRun = false;

	StartListener->IsEnabled = true;
	StopListener->IsEnabled = false;

	// After stop socket server thread, enable 'Connect to Listener' item
	rootPage->ScenarioItemEnable(1, true);
}

LocalHostItem::LocalHostItem(HostName^ localHostName)
{
    if (localHostName == nullptr)
    {
        throw ref new InvalidArgumentException("localHostName cannot be null");
    }

    if (localHostName->IPInformation == nullptr)
    {
        throw ref new InvalidArgumentException("Adapter information not found");
    }

    this->localHost = localHostName;
    this->displayString = "Address: " + localHostName->DisplayName +
        " Adapter: " + localHostName->IPInformation->NetworkAdapter->NetworkAdapterId;
}


void SDKTemplate::WiFiTestTools::WiFiServer::ClearReportMessages_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	ReportMessageBox->Text = L"";
}

void SDKTemplate::WiFiTestTools::WiFiServer::AppendReportMessage(Platform::String^ message)
{
	String^ str = ReportMessageBox->Text->ToString();
	String^ new_str = String::Concat(str, message);
	ReportMessageBox->Text = new_str;
	
}

void SDKTemplate::WiFiTestTools::WiFiServer::IperfThreadEnd()
{
	StartListener->IsEnabled = true;
	StopListener->IsEnabled = false;

	// After stop socket server thread, enable 'Connect to Listener' item
	rootPage->ScenarioItemEnable(1, true);
}