#include "MainPage.xaml.h"

using namespace SDKTemplate;

#define printf printf_report

extern void send_message_to_UI(char * message, NotifyType type);
extern void printf_report(const char *str, ... );