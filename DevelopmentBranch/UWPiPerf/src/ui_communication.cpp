#include "pch.h"

#include "ui_communication.h"

using namespace SDKTemplate;
using namespace Platform;
using namespace Windows::UI::Core;

void printf_report(const char *str, ...)
{
	char reportMessage[100];
	va_list ap;

	va_start(ap, str);

	memset(reportMessage, 0, 100);
	int n = vsprintf(reportMessage, str, ap);

	va_end(ap);

	send_message_to_UI(reportMessage, NotifyType::ReportMessage);
}

void send_message_to_UI(char * message, NotifyType type)
{
	int len = strlen(message);
	
	WCHAR *wstr = new WCHAR[len + 1];

	memset(wstr, 0, (len + 1)*sizeof(WCHAR));

	MultiByteToWideChar(CP_ACP, 0, message, len, wstr, len*2);

	String ^str;

	str = ref new String(wstr);

	MainPage::Current->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([str, type]()
	{
		MainPage::Current->NotifyUser(str, type);
	}));
}