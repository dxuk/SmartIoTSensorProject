using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.Common.Exceptions;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.Common.Helpers;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.Common.Models;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.Common.Models.Commands;

namespace Microsoft.Azure.Devices.Applications.RemoteMonitoring.Common.Factory
{
    public static class SampleDeviceFactory
    {
        public const string OBJECT_TYPE_DEVICE_INFO = "DeviceInfo";

        public const string VERSION_1_0 = "1.0";

        private const int MAX_COMMANDS_SUPPORTED = 6;

        private const bool IS_SIMULATED_DEVICE = true;

        private static readonly Random Rand = new Random();

        public static readonly List<string> DefaultCabinNames = new List<string>
        {
            "Cabin001", "Cabin002", "Cabin003", "Cabin004", "Cabin005", "Cabin006", "Cabin007", "Cabin008", "Cabin009", "Cabin010", "Cabin011"
            , "Cabin012", "Cabin013", "Cabin014", "Cabin015", "Cabin016", "Cabin017", "Cabin018", "Cabin019", "Cabin020", "Cabin021", "Cabin022"
            , "Cabin023", "Cabin024", "Cabin025", "Cabin026", "Cabin027", "Cabin028", "Cabin029", "Cabin030", "Cabin031", "Cabin032"
        };

        public static readonly List<string> DefaultCameraNames = new List<string>
        {
            "Camera001", "Camera002", "Camera003", "Camera004"
        };

        public static DeviceModel GetSampleCabinSimulatedDevice(string deviceId, string key)
        {
            DeviceModel device = DeviceCreatorHelper.BuildCabinDeviceStructure(deviceId, true, null);

            AssignCabinDeviceProperties(device);
            device.ObjectType = OBJECT_TYPE_DEVICE_INFO;
            device.Version = VERSION_1_0;
            device.IsSimulatedDevice = IS_SIMULATED_DEVICE;

            AssignCabinTelemetry(device);
            AssignCabinCommands(device);

            return device;
        }

        public static DeviceModel GetSampleCameraSimulatedDevice(string deviceId, string key)
        {
            DeviceModel device = DeviceCreatorHelper.BuildCameraDeviceStructure(deviceId, true, null);

            AssignCameraDeviceProperties(device);
            device.ObjectType = OBJECT_TYPE_DEVICE_INFO;
            device.Version = VERSION_1_0;
            device.IsSimulatedDevice = IS_SIMULATED_DEVICE;

            AssignCameraTelemetry(device);
            AssignCameraCommands(device);

            return device;
        }

        public static DeviceModel GetSampleCabinDevice(Random randomNumber, SecurityKeys keys)
        {
            var deviceId = string.Format(
                    CultureInfo.InvariantCulture,
                    "00000-DEV-{0}C-{1}LK-{2}D-{3}",
                    MAX_COMMANDS_SUPPORTED, 
                    randomNumber.Next(99999),
                    randomNumber.Next(99999),
                    randomNumber.Next(99999));

            var device = DeviceCreatorHelper.BuildCabinDeviceStructure(deviceId, false, null);
            device.ObjectName = "IoT Device Description";

            AssignCabinDeviceProperties(device);
            AssignCabinTelemetry(device);
            AssignCabinCommands(device);

            return device;
        }

        public static DeviceModel GetSampleCameraDevice(Random randomNumber, SecurityKeys keys)
        {
            var deviceId = string.Format(
                    CultureInfo.InvariantCulture,
                    "00000-DEV-{0}C-{1}LK-{2}D-{3}",
                    MAX_COMMANDS_SUPPORTED,
                    randomNumber.Next(99999),
                    randomNumber.Next(99999),
                    randomNumber.Next(99999));

            var device = DeviceCreatorHelper.BuildCameraDeviceStructure(deviceId, false, null);
            device.ObjectName = "IoT Device Description";

            AssignCameraDeviceProperties(device);
            AssignCameraTelemetry(device);
            AssignCameraCommands(device);

            return device;
        }

        private static void AssignCabinDeviceProperties(DeviceModel device)
        {
            if (device?.DeviceProperties == null)
            {
                throw new DeviceRequiredPropertyNotFoundException("Required DeviceProperties not found");
            }

            device.DeviceProperties.HubEnabledState = true;
            device.DeviceProperties.DeviceType = "Cabin Sensor";
        }

        private static void AssignCameraDeviceProperties(DeviceModel device)
        {
            if (device?.DeviceProperties == null)
            {
                throw new DeviceRequiredPropertyNotFoundException("Required DeviceProperties not found");
            }

            device.DeviceProperties.HubEnabledState = true;
            device.DeviceProperties.DeviceType = "Smart Camera";
        }

        private static void AssignCabinTelemetry(DeviceModel device)
        {
            device.Telemetry.Add(new Telemetry("Temperature", "Temperature", "double"));
            device.Telemetry.Add(new Telemetry("Humidity", "Humidity", "double"));
        }

        private static void AssignCameraTelemetry(DeviceModel device)
        {
            device.Telemetry.Add(new Telemetry("PeopleCount", "People Count", "int"));
        }

        private static void AssignCabinCommands(DeviceModel device)
        {
            device.Commands.Add(new Command("PingDevice"));
            device.Commands.Add(new Command("StartTelemetry"));
            device.Commands.Add(new Command("StopTelemetry"));
            device.Commands.Add(new Command("ChangeSetPointTemp", new [] { new Parameter("SetPointTemp", "double") }));
            device.Commands.Add(new Command("DiagnosticTelemetry", new[] { new Parameter("Active", "boolean") }));
            device.Commands.Add(new Command("ChangeDeviceState", new[] { new Parameter("DeviceState", "string") }));
        }

        private static void AssignCameraCommands(DeviceModel device)
        {
            device.Commands.Add(new Command("PingDevice"));
            device.Commands.Add(new Command("StartTelemetry"));
            device.Commands.Add(new Command("StopTelemetry"));
            device.Commands.Add(new Command("ChangeDeviceState", new[] { new Parameter("DeviceState", "string") }));
        }
    }
}
