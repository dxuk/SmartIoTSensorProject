﻿#pragma once
//------------------------------------------------------------------------------
//     This code was generated by a tool.
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
//------------------------------------------------------------------------------


namespace Windows {
    namespace UI {
        namespace Xaml {
            namespace Controls {
                ref class TextBlock;
                ref class TextBox;
                ref class Button;
            }
        }
    }
}
namespace Windows {
    namespace UI {
        namespace Xaml {
            ref class VisualState;
        }
    }
}

namespace SDKTemplate
{
    namespace WiFiTestTools
    {
        [::Windows::Foundation::Metadata::WebHostHidden]
        partial ref class WiFiServer : public ::Windows::UI::Xaml::Controls::Page, 
            public ::Windows::UI::Xaml::Markup::IComponentConnector,
            public ::Windows::UI::Xaml::Markup::IComponentConnector2
        {
        public:
            void InitializeComponent();
            virtual void Connect(int connectionId, ::Platform::Object^ target);
            virtual ::Windows::UI::Xaml::Markup::IComponentConnector^ GetBindingConnector(int connectionId, ::Platform::Object^ target);
        
        private:
            bool _contentLoaded;
        
            private: ::Windows::UI::Xaml::Controls::TextBlock^ InputTextBlock1;
            private: ::Windows::UI::Xaml::Controls::TextBox^ ServerPortForListener;
            private: ::Windows::UI::Xaml::Controls::TextBox^ TimeInterval;
            private: ::Windows::UI::Xaml::Controls::Button^ ClearReportMessages;
            private: ::Windows::UI::Xaml::Controls::TextBox^ ReportMessageBox;
            private: ::Windows::UI::Xaml::Controls::Button^ StartListener;
            private: ::Windows::UI::Xaml::Controls::Button^ StopListener;
            private: ::Windows::UI::Xaml::VisualState^ DefaultLayout;
            private: ::Windows::UI::Xaml::VisualState^ Below768Layout;
        };
    }
}

