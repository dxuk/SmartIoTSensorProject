using Microsoft.Azure.Devices.Applications.RemoteMonitoring.Simulator.WebJob.Cabin.Telemetry;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.Simulator.WebJob.SimulatorCore.Devices;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.Simulator.WebJob.SimulatorCore.Logging;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.Simulator.WebJob.Cabin.Telemetry.Factory;
using Moq;
using Xunit;
using Ploeh.AutoFixture;

namespace Microsoft.Azure.Devices.Applications.RemoteMonitoring.UnitTests.Simulator.WebJob
{
    class CabinTelemetryFactoryTests
    {
        private CabinTelemetryFactory factory;
        private Mock<ILogger> _loggerMock;
        private IFixture _fixture;

        public CabinTelemetryFactoryTests()
        {
            this._loggerMock = new Mock<ILogger>();
            this.factory = new CabinTelemetryFactory(this._loggerMock.Object);
            this._fixture = new Fixture();
        }

        [Fact]
        public void PopulateDeviceWithTelemetryEventsTest()
        {
            DeviceBase device = this._fixture.Create<DeviceBase>();

            this.factory.PopulateDeviceWithTelemetryEvents(device);

            Assert.Equal(device.TelemetryEvents.Count, 2);
            Assert.IsType<StartupTelemetry>(device.TelemetryEvents[0]);
            Assert.IsType<RemoteMonitorTelemetry>(device.TelemetryEvents[1]);
        }
    }
}
