using System;
using System.Collections.Generic;
using System.Linq;
using System.Web.Http;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.Common.Configurations;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.Common.Models;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.DeviceAdmin.Infrastructure.BusinessLogic;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.DeviceAdmin.Infrastructure.Models;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.DeviceAdmin.Web.Models;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.DeviceAdmin.Web.WebApiControllers;
using Moq;
using Ploeh.AutoFixture;
using Xunit;

namespace Microsoft.Azure.Devices.Applications.RemoteMonitoring.UnitTests.Web.
    WebApiControllers
{
    public class TelemetryApiControllerTests
    {
        private readonly Mock<IAlertsLogic> alertLogic;
        private readonly Mock<IConfigurationProvider> configProvider;
        private readonly Mock<IDeviceLogic> deviceLogic;
        private readonly IFixture fixture;
        private readonly TelemetryApiController telemetryApiController;
        private readonly Mock<IDeviceTelemetryLogic> telemetryLogic;

        public TelemetryApiControllerTests()
        {
            telemetryLogic = new Mock<IDeviceTelemetryLogic>();
            alertLogic = new Mock<IAlertsLogic>();
            deviceLogic = new Mock<IDeviceLogic>();
            configProvider = new Mock<IConfigurationProvider>();
            telemetryApiController = new TelemetryApiController(telemetryLogic.Object,
                alertLogic.Object,
                deviceLogic.Object,
                configProvider.Object);
            telemetryApiController.InitializeRequest();
            fixture = new Fixture();
        }

        [Fact]
        public async void GetDashboardDevicePaneDataAsyncTest()
        {
            var deviceID = fixture.Create<string>();
            var device = fixture.Create<DeviceModel>();
            var telemetryFields = fixture.Create<IList<DeviceTelemetryFieldModel>>();
            var telemetryModel = fixture.Create<IEnumerable<DeviceTelemetryModel>>();
            var summModel = fixture.Create<DeviceTelemetrySummaryModel>();
            deviceLogic.Setup(mock => mock.GetDeviceAsync(deviceID)).ReturnsAsync(device);
            deviceLogic.Setup(mock => mock.ExtractTelemetry(device)).Returns(telemetryFields);

            telemetryLogic.Setup(mock => mock.LoadLatestDeviceTelemetrySummaryAsync(deviceID, It.IsAny<DateTime>()))
                .ReturnsAsync(summModel);
            telemetryLogic.Setup(
                mock => mock.LoadLatestDeviceTelemetryAsync(deviceID, telemetryFields, It.IsAny<DateTime>()))
                .ReturnsAsync(telemetryModel);

            var res = await telemetryApiController.GetDashboardDevicePaneDataAsync(deviceID);
            res.AssertOnError();
            var data = res.ExtractContentAs<DashboardDevicePaneDataModel>();
            Assert.Equal(data.DeviceTelemetryFields, telemetryFields.ToArray());
            Assert.Equal(data.DeviceTelemetrySummaryModel, summModel);
            Assert.Equal(data.DeviceTelemetryModels, telemetryModel.OrderBy(t => t.Timestamp).ToArray());

            deviceLogic.Setup(mock => mock.ExtractTelemetry(device)).Throws(new Exception());
            await
                Assert.ThrowsAsync<HttpResponseException>(
                    () => telemetryApiController.GetDashboardDevicePaneDataAsync(deviceID));
        }

        [Fact]
        public async void GetDeviceTelemetryAsyncTest()
        {
            var deviceId = fixture.Create<string>();
            var minTime = fixture.Create<DateTime>();
            var device = fixture.Create<DeviceModel>();
            var telemetryModel = fixture.Create<List<DeviceTelemetryModel>>();
            fixture.Create<IEnumerable<DeviceTelemetryModel>>();
            var telemetryFields = fixture.Create<IList<DeviceTelemetryFieldModel>>();

            deviceLogic.Setup(mock => mock.GetDeviceAsync(deviceId)).ReturnsAsync(device);
            deviceLogic.Setup(mock => mock.ExtractTelemetry(device)).Returns(telemetryFields);
            telemetryLogic.Setup(mock => mock.LoadLatestDeviceTelemetryAsync(deviceId, telemetryFields, minTime))
                .ReturnsAsync(telemetryModel);

            var res = await telemetryApiController.GetDeviceTelemetryAsync(deviceId, minTime);
            res.AssertOnError();
            var data = res.ExtractContentAs<DeviceTelemetryModel[]>();
            Assert.Equal(data, telemetryModel.OrderBy(t => t.Timestamp).ToArray());

            deviceLogic.Setup(mock => mock.ExtractTelemetry(device)).Throws(new Exception());
            await
                Assert.ThrowsAsync<HttpResponseException>(
                    () => telemetryApiController.GetDeviceTelemetryAsync(deviceId, minTime));
            res.AssertOnError();
        }

        [Fact]
        public async void GetDeviceTelemetrySummaryAsyncTest()
        {
            var telemetrySummary = fixture.Create<DeviceTelemetrySummaryModel>();
            var deviceId = fixture.Create<string>();
            telemetryLogic.Setup(mock => mock.LoadLatestDeviceTelemetrySummaryAsync(deviceId, null))
                .ReturnsAsync(telemetrySummary);
            var res = await telemetryApiController.GetDeviceTelemetrySummaryAsync(deviceId);
            res.AssertOnError();
            var data = res.ExtractContentAs<DeviceTelemetrySummaryModel>();
            Assert.Equal(data, telemetrySummary);
        }

        [Fact]
        public async void GetMapApiKeyTest()
        {
            var key = fixture.Create<string>();
            configProvider.Setup(mock => mock.GetConfigurationSettingValue("MapApiQueryKey")).Returns(key);
            var res = await telemetryApiController.GetMapApiKey();
            res.AssertOnError();
            var data = res.ExtractContentAs<string>();
            Assert.Equal(data, key);

            configProvider.Setup(mock => mock.GetConfigurationSettingValue("MapApiQueryKey")).Returns("0");
            res = await telemetryApiController.GetMapApiKey();
            res.AssertOnError();
            data = res.ExtractContentAs<string>();
            Assert.Equal(data, string.Empty);
        }
    }
}