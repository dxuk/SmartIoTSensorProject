using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Microsoft.Azure.Devices.Applications.RemoteMonitoring.DeviceAdmin.Web.Models
{
    public class CurrentWeatherModel
    {
        public string Weather { get; set; }

        public string WindSpeed { get; set; }

        public string Temperature { get; set; }
    }
}