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
// WiFiServer.xaml.h
// Declaration of the WiFiServer class
//

#pragma once

#include "pch.h"
#include "WiFiServer.g.h"
#include "MainPage.xaml.h"

namespace SDKTemplate
{
    namespace WiFiTestTools
    {
        [Windows::Foundation::Metadata::WebHostHidden]
        // In c++, adding this attribute to ref classes enables data binding.
        // For more info search for 'Bindable' on the page http://go.microsoft.com/fwlink/?LinkId=254639 
        [Windows::UI::Xaml::Data::Bindable]
        public ref class LocalHostItem sealed
        {
        public:
            LocalHostItem(Windows::Networking::HostName^ localHostName);
            property Platform::String^ DisplayString { Platform::String^ get() { return displayString; } }
            property Windows::Networking::HostName^ LocalHost { Windows::Networking::HostName^ get() { return localHost; } }

        private:
            Platform::String^ displayString;
            Windows::Networking::HostName^ localHost;
        };

        /// <summary>
        /// An empty page that can be used on its own or navigated to within a Frame.
        /// </summary>
        [Windows::Foundation::Metadata::WebHostHidden]
        public ref class WiFiServer sealed
        {
        public:
            WiFiServer();
			void AppendReportMessage(Platform::String^ message);
			void IperfThreadEnd();

        protected:
            virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
        private:
            MainPage^ rootPage;
			HANDLE mThreadHandle;
			bool   mIperfIsRun;

            // Vector containing all available local HostName endpoints.
            Platform::Collections::Vector<LocalHostItem^>^ localHostItems;
            void StartListener_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
			void ClearReportMessages_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
			void StopListener_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		};
    }
}
