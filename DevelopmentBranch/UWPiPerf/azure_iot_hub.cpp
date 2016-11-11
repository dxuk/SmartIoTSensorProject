//#include "stdafx.h"
#include "pch.h"
#include <string>
#include <future>
#include <stdio.h>

#include "azure_c_shared_utility/platform.h"
#include "iothub_client.h"
#include "iothubtransportamqp.h"

#include <cvt/wstring>
#include <codecvt>

using namespace Platform;
using namespace Windows::Foundation;
//
// String containing Hostname, Device Id & Device Key in the format:
// "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"
//
// Note: this connection string is specific to the device "myFirstDevice". To configure other devices,
// see information on iothub-explorer at http://aka.ms/iothubgetstartedVSCS
//

// Use a Server or Client device string for each role.
//static const char* connection_string = "HostName=paulfotest.azure-devices.net;DeviceId=myFirstDevice;SharedAccessKey=QqEPhyCus/CjeRBJCMiGykVifYNhJVIgQ01HFwQ4F30=";
//static const char* connection_string = "HostName=paulfotest.azure-devices.net;DeviceId=myFirstDevice;SharedAccessKey=QqEPhyCus/CjeRBJCMiGykVifYNhJVIgQ01HFwQ4F30=";

//
// To monitor messages sent to device "myFirstDevice" use iothub-explorer as follows:
//    iothub-explorer HostName=paulfotest.azure-devices.net;SharedAccessKeyName=service;SharedAccessKey=V2PR5Yjtl1Bzc6DhbDKuopaRtOkhA1pbeOc61q9yyq8= monitor-events "myFirstDevice"
//

// Refer to http://aka.ms/azure-iot-hub-vs-cs-wiki for more information on Connected Service for Azure IoT Hub

struct callback_parameter
{
    std::string &message;
    std::promise<void> completion;
};

void putConnectionString(Platform::String^ deviceId, Platform::String^ sharedAccessKey, Platform::String^ hostName)
{
	// Local Settings
	Windows::Storage::ApplicationDataContainer^ localSettings = Windows::Storage::ApplicationData::Current->LocalSettings;

	// Create a simple setting

	auto values = localSettings->Values;
	values->Insert("DeviceId", dynamic_cast<PropertyValue^>(PropertyValue::CreateString(deviceId)));
	values->Insert("SharedAccessKey", dynamic_cast<PropertyValue^>(PropertyValue::CreateString(sharedAccessKey)));
	values->Insert("HostName", dynamic_cast<PropertyValue^>(PropertyValue::CreateString(hostName)));
}

const char* getConstructionString()
{
	// Construct connection string
	// Read data from a simple setting

	// Local Settings
	Windows::Storage::ApplicationDataContainer^ localSettings = Windows::Storage::ApplicationData::Current->LocalSettings;

	auto values = localSettings->Values;

	String^ deviceId = safe_cast<String^>(localSettings->Values->Lookup("DeviceId"));
	String^ SharedAccessKey = safe_cast<String^>(localSettings->Values->Lookup("SharedAccessKey"));
	String^ HostName = safe_cast<String^>(localSettings->Values->Lookup("HostName"));

	if (!deviceId || !SharedAccessKey || !HostName)
	{
		// No data
		return NULL;
	}


	//connection_string = "HostName=paulfotest.azure-devices.net;DeviceId=myFirstDevice;SharedAccessKey=QqEPhyCus/CjeRBJCMiGykVifYNhJVIgQ01HFwQ4F30=";

	Platform::String^ connection_string_P = (L"HostName=") + HostName + (L";DeviceId=") + deviceId + (L";SharedAccessKey=") + SharedAccessKey;

	// Convert Platform::String^ to std::string. 
	// Platform::String^ is wide characters
	stdext::cvt::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
	std::string connection_string_S = convert.to_bytes(connection_string_P->Data());
	const char * connection_string = connection_string_S.c_str();



	return connection_string;
}

void send_callback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* context)
{
    auto callback_param = (callback_parameter*)context;

    printf("Message '%s' Received. Result is: %d\n", callback_param->message.c_str(), result);

    callback_param->completion.set_value();
}

void send_device_to_cloud_message(Platform::String^ payload)
{
	if (platform_init() != 0)
	{
		printf("Failed initializing platform.\r\n");
		return;
	}

	const char* connection_string = getConstructionString();

	if (sizeof(connection_string) == 0)
	{
		return;
	}

    // Setup IoTHub client configuration
    IOTHUB_CLIENT_HANDLE iothub_client_handle = IoTHubClient_CreateFromConnectionString(connection_string, AMQP_Protocol);
    if (iothub_client_handle == nullptr)
    {
        printf("Failed on IoTHubClient_Create\r\n");
    }
    else
    {
		// Convert Platform::String^ to std::string. 
		// Platform::String^ is wide characters
		stdext::cvt::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
		std::string message = convert.to_bytes(payload->Data());

		//std::string message = "Hello, Cloud from C++!";

        IOTHUB_MESSAGE_HANDLE message_handle = IoTHubMessage_CreateFromByteArray((const unsigned char*)message.data(), message.size());
        if (message_handle == nullptr)
        {
            printf("unable to create a new IoTHubMessage\n");
        }
        else
        {
            callback_parameter callback_param = { message };
            if (IoTHubClient_SendEventAsync(iothub_client_handle, message_handle, send_callback, &callback_param) != IOTHUB_CLIENT_OK)
            {
                printf("failed to hand over the message to IoTHubClient");
            }
            else
            {
                printf("IoTHubClient accepted the message for delivery\n");
            }

            IoTHubMessage_Destroy(message_handle);
            callback_param.completion.get_future().wait();
        }

        printf("Done!\n");
    }
    IoTHubClient_Destroy(iothub_client_handle);

    platform_deinit();
}

static IOTHUBMESSAGE_DISPOSITION_RESULT receive_callback(IOTHUB_MESSAGE_HANDLE message, void* context)
{
    auto completion = (std::promise<void>*)context;

    const unsigned char* buffer;
    size_t size;
    if (IoTHubMessage_GetByteArray(message, &buffer, &size) != IOTHUB_MESSAGE_OK)
    {
        printf("unable to IoTHubMessage_GetByteArray\r\n");
    }
    else
    {
        /*buffer is not zero terminated*/
        std::string str_msg;
        str_msg.resize(size + 1);

        memcpy((void*)str_msg.data(), buffer, size);
        str_msg[size] = '\0';

        printf("Received message '%s' from IoT Hub\n", str_msg.c_str());
    }

    completion->set_value();

    return IOTHUBMESSAGE_ACCEPTED;
}

void receive_cloud_to_device_message()
{
    if (platform_init() != 0)
    {
        printf("Failed initializing platform.\r\n");
        return;
    }

	const char* connection_string = getConstructionString();

    IOTHUB_CLIENT_HANDLE iothub_client_handle = IoTHubClient_CreateFromConnectionString(connection_string, AMQP_Protocol);
    if (iothub_client_handle == nullptr)
    {
        printf("Failed on IoTHubClient_Create\r\n");
    }
    else
    {
        std::promise<void> completion;
        if (IoTHubClient_SetMessageCallback(iothub_client_handle, receive_callback, &completion) != IOTHUB_CLIENT_OK)
        {
            printf("unable to IoTHubClient_SetMessageCallback\r\n");
        }

        completion.get_future().wait();
        IoTHubClient_Destroy(iothub_client_handle);
    }

    platform_deinit();
}

