﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Microsoft.Azure.Devices.Applications.RemoteMonitoring.DeviceAdmin.Web.Models
{
    public class CurrentWeatherResultsModel
    {
        public List<CurrentWeatherModel> Data { get; set; }
    }
}