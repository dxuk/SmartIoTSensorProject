using System;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.Common.Factory;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.Common.Helpers;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.DeviceAdmin.Infrastructure.BusinessLogic;
using Xunit;

namespace Microsoft.Azure.Devices.Applications.RemoteMonitoring.UnitTests.Common
{
    public class SampleDeviceFactoryTests
    {
        [Fact]
        public void TestGetDefaultCabinDeviceNames()
        {
            var s = SampleDeviceFactory.DefaultCabinNames;
            Assert.NotEmpty(s);
        }

        [Fact]
        public void TestGetCabinSampleDevice()
        {
            var randomnumber = new Random();
            ISecurityKeyGenerator securityKeyGenerator = new SecurityKeyGenerator();
            var keys = securityKeyGenerator.CreateRandomKeys();
            var d = SampleDeviceFactory.GetSampleCabinDevice(randomnumber, keys);
            Assert.NotNull(d);
            Assert.NotNull(d.DeviceProperties);
            Assert.NotNull(d.DeviceProperties.DeviceID);
        }

        [Fact]
        public void TestGetCabinSampleSimulatedDevice()
        {
            var d = DeviceCreatorHelper.BuildCabinDeviceStructure("test", true, null);
            Assert.NotNull(d);
            Assert.Equal("test", d.DeviceProperties.DeviceID);
            Assert.Equal("normal", d.DeviceProperties.DeviceState);
            Assert.Equal(null, d.DeviceProperties.HubEnabledState);
        }

        [Fact]
        public void TestGetDefaultCameraDeviceNames()
        {
            var s = SampleDeviceFactory.DefaultCameraNames;
            Assert.NotEmpty(s);
        }

        [Fact]
        public void TestGetCameraSampleDevice()
        {
            var randomnumber = new Random();
            ISecurityKeyGenerator securityKeyGenerator = new SecurityKeyGenerator();
            var keys = securityKeyGenerator.CreateRandomKeys();
            var d = SampleDeviceFactory.GetSampleCameraDevice(randomnumber, keys);
            Assert.NotNull(d);
            Assert.NotNull(d.DeviceProperties);
            Assert.NotNull(d.DeviceProperties.DeviceID);
        }

        [Fact]
        public void TestGetCameraSampleSimulatedDevice()
        {
            var d = DeviceCreatorHelper.BuildCameraDeviceStructure("test", true, null);
            Assert.NotNull(d);
            Assert.Equal("test", d.DeviceProperties.DeviceID);
            Assert.Equal("normal", d.DeviceProperties.DeviceState);
            Assert.Equal(null, d.DeviceProperties.HubEnabledState);
        }
    }
}