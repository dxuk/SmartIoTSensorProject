using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Microsoft.Azure.Devices.Applications.RemoteMonitoring.DeviceAdmin.Web.Models
{
    public class AlertHistoryDeviceModel
    {
        public string DeviceId
        {
            get;
            set;
        }

        public AlertHistoryDeviceStatus Status
        {
            get;
            set;
        }
    }

    public enum AlertHistoryDeviceStatus
    {
        AllClear = 0,
        Caution,
        Critical
    }
}