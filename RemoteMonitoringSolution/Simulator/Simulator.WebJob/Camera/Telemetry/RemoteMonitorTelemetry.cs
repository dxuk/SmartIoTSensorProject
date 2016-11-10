using System;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.Common.SampleDataGenerator;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.Simulator.WebJob.Camera.Telemetry.Data;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.Simulator.WebJob.SimulatorCore.Logging;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.Simulator.WebJob.SimulatorCore.Telemetry;

namespace Microsoft.Azure.Devices.Applications.RemoteMonitoring.Simulator.WebJob.Camera.Telemetry
{
    public class RemoteMonitorTelemetry : ITelemetry
    {
        private readonly ILogger _logger;
        private readonly string _deviceId;

        private const int REPORT_FREQUENCY_IN_SECONDS = 60;

        private SampleDataGenerator _peopleCountGenerator;

        public bool TelemetryActive { get; set; }

        public RemoteMonitorTelemetry(ILogger logger, string deviceId)
        {
            _logger = logger;
            _deviceId = deviceId;

            TelemetryActive = true;

            _peopleCountGenerator = new SampleDataGenerator(0, 1000);
        }

        public async Task SendEventsAsync(CancellationToken token, Func<object, Task> sendMessageAsync)
        {
            var monitorData = new RemoteMonitorTelemetryData();
            string messageBody;
            while (!token.IsCancellationRequested)
            {
                if (TelemetryActive)
                {
                    monitorData.DeviceId = _deviceId;
                    monitorData.PeopleCount = (int)Math.Round(_peopleCountGenerator.GetNextValue(), 0);
                    messageBody = "PeopleCount: " + monitorData.PeopleCount;

                    await sendMessageAsync(monitorData);
                }
                await Task.Delay(TimeSpan.FromSeconds(REPORT_FREQUENCY_IN_SECONDS), token);
            }
        }
    }
}
