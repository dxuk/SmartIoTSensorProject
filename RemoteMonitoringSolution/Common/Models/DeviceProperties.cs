using System;

namespace Microsoft.Azure.Devices.Applications.RemoteMonitoring.Common.Models
{
    public class DeviceProperties
    {
        public string DeviceID { get; set; }
        public bool? HubEnabledState { get; set; }
        public DateTime? CreatedTime { get; set; }
        public string DeviceState { get; set; }
        public DateTime? UpdatedTime { get; set; }
        public string DeviceType { get; set; }

        public override string ToString()
        {
            return Newtonsoft.Json.JsonConvert.SerializeObject(this);
        }

        public bool GetHubEnabledState()
        {
            return HubEnabledState.HasValue && HubEnabledState.Value;
        }
    }
}