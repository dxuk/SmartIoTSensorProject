using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Microsoft.Azure.Devices.Applications.RemoteMonitoring.DeviceAdmin.Web.Models
{
    public class DailyForecastModel
    {
        public string Date { get; set; }

        public string Weather { get; set; }
    }
}