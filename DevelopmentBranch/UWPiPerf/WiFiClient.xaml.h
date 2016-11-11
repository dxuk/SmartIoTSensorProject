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
// WiFiClient.xaml.h
// Declaration of the WiFiClient class
//

#pragma once

#include "pch.h"
#include "WiFiclient.g.h"
#include "MainPage.xaml.h"

namespace SDKTemplate
{
    namespace WiFiTestTools
    {
        /// <summary>
        /// An empty page that can be used on its own or navigated to within a Frame.
        /// </summary>
        [Windows::Foundation::Metadata::WebHostHidden]
        public ref class WiFiClient sealed
        {
        public:
            WiFiClient();
			void AppendReportMessage(Platform::String^ message);
			void IperfThreadEnd();

        protected:
            virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
        private:
            MainPage^ rootPage;
			bool      mIperfIsRun;

            Windows::Networking::Connectivity::NetworkAdapter^ adapter;
            void ConnectSocket_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
			void ClearReportMessages_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
			void StopConnect_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		};
    }
}
