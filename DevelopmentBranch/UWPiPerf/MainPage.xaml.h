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

#pragma once

#include "MainPage.g.h"
#include "Configuration.h"

namespace SDKTemplate
{
    public enum class NotifyType
    {
        StatusMessage,
        ErrorMessage,
		ReportMessage,
		IperfThreadEnd
    };

	public enum class FrameType
	{
		WiFiServer,
		WiFiClient
	};

    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public ref class MainPage sealed
    {
    public:
        MainPage();
	private:
		~MainPage();

    protected:
        virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;

    private:
        void ScenarioControl_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
        void Footer_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

    internal:
        static MainPage^ Current;
		static Object^ ChildFrame;
		static FrameType ChildType;
        void NotifyUser(Platform::String^ strMessage, NotifyType type);
	public:
		void ScenarioItemEnable(int index, bool enable);
    };
}
