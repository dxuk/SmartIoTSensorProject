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
// WiFiClient.xaml.cpp
// Implementation of the WiFiClient class
//

#include "pch.h"
#include "WiFiClient.xaml.h"

#include "socket_threads.h"

using namespace SDKTemplate;
using namespace SDKTemplate::WiFiTestTools;

using namespace Concurrency;
using namespace Platform;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Networking;
using namespace Windows::Networking::Connectivity;
using namespace Windows::Networking::Sockets;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;

WiFiClient::WiFiClient() : adapter(nullptr)
{
    InitializeComponent();
	mIperfIsRun = false;
	ConnectSocket->IsEnabled = true;
	StopConnect->IsEnabled = false;
}

/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>
void WiFiClient::OnNavigatedTo(NavigationEventArgs^ e)
{
    // A pointer back to the main page.  This is needed if you want to call methods in MainPage such
    // as NotifyUser()
    rootPage = MainPage::Current;
	MainPage::ChildFrame = this;
	MainPage::ChildType = FrameType::WiFiClient;

    if (CoreApplication::Properties->HasKey("serverAddress"))
    {
        Object^ serverAddressObject = CoreApplication::Properties->Lookup("serverAddress");
        String^ serverAddress = dynamic_cast<String^>(serverAddressObject);
        if (serverAddress)
        {
            HostNameForConnect->Text = serverAddress;
        }
    }

    adapter = nullptr;
    if (CoreApplication::Properties->HasKey("adapter"))
    {
        Object^ networkAdapter = CoreApplication::Properties->Lookup("adapter");
        adapter = dynamic_cast<NetworkAdapter^>(networkAdapter);
    }
}


void WiFiClient::ConnectSocket_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    // Overriding the socket here is safe as it will be deleted once all references to it are gone. However, in many
    // cases this is a dangerous pattern to override data semi-randomly (each time user clicked the button) so we
    // block it here.
	int paramc = 0;

	// for HostName
	char host_param[65] = { 0 };
	char host_name[70] = { 0 };

	// for port
	char port_param[15] = { 0 };
	char port[10] = { 0 };

	// for TCP UDP option
	char udp_param[] = "-u";

	// for IPv4 or IPv6 option
	char ipv4v6_param[5] = { 0 };

	// for TOS
	char tos_param[10] = { 0 };

	// for Time Duration
	char duration_param[15] = { 0 };
	char duration[10] = { 0 };

	// for time interval
	char interval[10] = { 0 };
	char interval_param[15] = { 0 };

	// for Parallel Number
	char parallel[10] = { 0 };
	char parallel_param[15] = { 0 };

	// for Socket Buffer Size
	char buffer_size[10] = { 0 };
	char buffer_size_param[15] = { 0 };

	// for MSS (MTU - 40)
	char mss[10] = { 0 };
	char mss_param[15] = { 0 };

	// for Omit Time
	char omit_time[10] = { 0 };
	char omit_time_param[15] = { 0 };

	// for Reverse mode
	char nodelay_param[] = "-N";


	char * params[20];

	// get Host Name
	if (HostNameForConnect->Text != nullptr && HostNameForConnect->Text->Length() < 64)
	{
		WideCharToMultiByte(CP_ACP, // ANSI Code Page
			0, // No special handling of unmapped chars
			HostNameForConnect->Text->Data(), // wide-character string to be converted
			HostNameForConnect->Text->Length(),
			host_name,
			64,
			"5001", NULL); // Unrepresented char replacement - Use Default
		sprintf(host_param, "-c %s", host_name);
		params[paramc] = host_param;
		paramc++;
	}

	// get port
	if (ServiceNameForConnect->Text != nullptr && ServiceNameForConnect->Text->Length() < 10)
	{
		WideCharToMultiByte(CP_ACP, // ANSI Code Page
			0, // No special handling of unmapped chars
			ServiceNameForConnect->Text->Data(), // wide-character string to be converted
			ServiceNameForConnect->Text->Length(),
			port,
			9,
			"5001", NULL); // Unrepresented char replacement - Use Default
		sprintf(port_param, "-p %s", port);
		params[paramc] = port_param;
		paramc++;
	}

	// get TCP UDP optoin
	if (((ComboBoxItem ^)cbTcpUdp->SelectedItem)->Content->ToString()->Equals("UDP"))
	{
		params[paramc] = udp_param;
		paramc++;
	}

	// get IPv4 IPv6 optoin
	if (((ComboBoxItem ^)cbIPv4v6->SelectedItem)->Content->ToString()->Equals("IPv4"))
	{
		strncpy(ipv4v6_param, "-4", 2);
		params[paramc] = ipv4v6_param;
		paramc++;
	}
	else if (((ComboBoxItem ^)cbIPv4v6->SelectedItem)->Content->ToString()->Equals("IPv6"))
	{
		strncpy(ipv4v6_param, "-6", 2);
		params[paramc] = ipv4v6_param;
		paramc++;
	}

	// get tos
	if (((ComboBoxItem ^)TypeOfService->SelectedItem)->Content->ToString()->Equals("Minimize Delay"))
	{
		strncpy(tos_param, "-S 16", 5);
		params[paramc] = tos_param;
		paramc++;
	}
	else if (((ComboBoxItem ^)TypeOfService->SelectedItem)->Content->ToString()->Equals("Maximize Throughput"))
	{
		strncpy(tos_param, "-S 8", 4);
		params[paramc] = tos_param;
		paramc++;
	}
	else if (((ComboBoxItem ^)TypeOfService->SelectedItem)->Content->ToString()->Equals("Maximize Reliability"))
	{
		strncpy(tos_param, "-S 4", 4);
		params[paramc] = tos_param;
		paramc++;
	}
	else if (((ComboBoxItem ^)TypeOfService->SelectedItem)->Content->ToString()->Equals("Minimize Cost"))
	{
		strncpy(tos_param, "-S 2", 4);
		params[paramc] = tos_param;
		paramc++;
	}

	// get time duration
	if (TimeDuration->Text != nullptr && TimeDuration->Text->Length() < 10)
	{
		WideCharToMultiByte(CP_ACP, // ANSI Code Page
			0, // No special handling of unmapped chars
			TimeDuration->Text->Data(), // wide-character string to be converted
			TimeDuration->Text->Length(),
			duration,
			9,
			"10", NULL); // Unrepresented char replacement - Use Default
		sprintf(duration_param, "-t %s", duration);
		params[paramc] = duration_param;
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

	// get Parallel Number
	if (ParallelNumber->Text != nullptr && ParallelNumber->Text->Length() < 10)
	{
		WideCharToMultiByte(CP_ACP, // ANSI Code Page
			0, // No special handling of unmapped chars
			ParallelNumber->Text->Data(), // wide-character string to be converted
			ParallelNumber->Text->Length(),
			parallel,
			9,
			"2", NULL); // Unrepresented char replacement - Use Default
		sprintf(parallel_param, "-P %s", parallel);
		params[paramc] = parallel_param;
		paramc++;
	}

	// get Socket Buffer Size
	if (SocketBufferSize->Text != nullptr && SocketBufferSize->Text->Length() < 10)
	{
		if (!SocketBufferSize->Text->Equals("0"))
		{
			WideCharToMultiByte(CP_ACP, // ANSI Code Page
				0, // No special handling of unmapped chars
				SocketBufferSize->Text->Data(), // wide-character string to be converted
				SocketBufferSize->Text->Length(),
				buffer_size,
				9,
				"2", NULL); // Unrepresented char replacement - Use Default
			sprintf(buffer_size_param, "-w %s", buffer_size);
			params[paramc] = buffer_size_param;
			paramc++;
		}
	}

	// get MSS (MTU - 40)
	if (TcpMSS->Text != nullptr && TcpMSS->Text->Length() < 10)
	{
		if (!TcpMSS->Text->Equals("0"))
		{
			WideCharToMultiByte(CP_ACP, // ANSI Code Page
				0, // No special handling of unmapped chars
				TcpMSS->Text->Data(), // wide-character string to be converted
				TcpMSS->Text->Length(),
				mss,
				9,
				"2", NULL); // Unrepresented char replacement - Use Default
			sprintf(mss_param, "-M %s", mss);
			params[paramc] = mss_param;
			paramc++;
		}
	}

	// get No Delay Mode
	if (NoDelayMode->IsChecked->Value)
	{
		params[paramc] = nodelay_param;
		paramc++;
	}


	// TODO
	// Initialise the IoT Hub payload for new session

	StartIPerfThread(paramc, params);

	ReportMessageBox->Text = L"";

	mIperfIsRun = true;

	ConnectSocket->IsEnabled = false;
	StopConnect->IsEnabled = true;

	// After start socket server thread, disable 'Connect to Listener' item
	rootPage->ScenarioItemEnable(0, false);
}


void SDKTemplate::WiFiTestTools::WiFiClient::StopConnect_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	EndIPerfThread(false);

	mIperfIsRun = false;
	ConnectSocket->IsEnabled = true;
	StopConnect->IsEnabled = false;

	// After stop socket server thread, enable 'Connect to Listener' item
	rootPage->ScenarioItemEnable(0, true);
}


void SDKTemplate::WiFiTestTools::WiFiClient::ClearReportMessages_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	ReportMessageBox->Text = L"";
}

void SDKTemplate::WiFiTestTools::WiFiClient::AppendReportMessage(Platform::String^ message)
{
	String^ str = ReportMessageBox->Text->ToString();
	String^ new_str = String::Concat(str, message);
	ReportMessageBox->Text = new_str;
}

void SDKTemplate::WiFiTestTools::WiFiClient::IperfThreadEnd()
{

	ConnectSocket->IsEnabled = true;
	StopConnect->IsEnabled = false;

	// After stop socket server thread, enable 'Connect to Listener' item
	rootPage->ScenarioItemEnable(0, true);
}