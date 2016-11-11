//
// IoTHub.xaml.cpp
// Implementation of the IoTHub class
//

#include "pch.h"
#include "IoTHub.xaml.h"
#include "azure_iot_hub.h"

using namespace SDKTemplate;
using namespace SDKTemplate::WiFiTestTools;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

bool dirty = false;

IoTHub::IoTHub()
{
	InitializeComponent();
}

/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>
void IoTHub::OnNavigatedTo(NavigationEventArgs^ e)
{
	// Construct connection string
	// Read data from a simple setting

	// Local Settings
	Windows::Storage::ApplicationDataContainer^ localSettings = Windows::Storage::ApplicationData::Current->LocalSettings;

	auto values = localSettings->Values;

	tbDeviceId->Text = safe_cast<String^>(localSettings->Values->Lookup("DeviceId"));
	tbSharedAccessKey->Text = safe_cast<String^>(localSettings->Values->Lookup("SharedAccessKey"));
	tbHostName->Text = safe_cast<String^>(localSettings->Values->Lookup("HostName"));


}

void IoTHub::OnNavigatedFrom(NavigationEventArgs ^ e)
{
	if (dirty)
	{
		putConnectionString(tbDeviceId->Text, tbSharedAccessKey->Text, tbHostName->Text);
	}
}

void SDKTemplate::WiFiTestTools::IoTHub::tbDeviceId_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
	dirty = true;
}


void SDKTemplate::WiFiTestTools::IoTHub::tbSharedAccessKey_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
	dirty = true;
}


void SDKTemplate::WiFiTestTools::IoTHub::tbHostName_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
	dirty = true;
}
