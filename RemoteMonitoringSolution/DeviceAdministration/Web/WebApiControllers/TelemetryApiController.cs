using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using System.Threading.Tasks;
using System.Web.Http;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.DeviceAdmin.Infrastructure.BusinessLogic;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.DeviceAdmin.Infrastructure.Models;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.DeviceAdmin.Web.Models;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.DeviceAdmin.Web.Security;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.Common.Configurations;
using GlobalResources;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.Common.Models;

using System.Text;
using System.Net;
using System.Web;
using Newtonsoft.Json.Linq;

using System.Runtime.Serialization;

using System.Runtime.Serialization.Json;
using System.IO;

namespace Microsoft.Azure.Devices.Applications.RemoteMonitoring.DeviceAdmin.Web.WebApiControllers
{

    /// <summary>
    /// A WebApiControllerBase-derived class for telemetry-related end points.
    /// </summary>
    [RoutePrefix("api/v1/telemetry")]
    public class TelemetryApiController : WebApiControllerBase
    {
        private const double MAX_DEVICE_SUMMARY_AGE_MINUTES = 10.0;
        private const int DISPLAYED_HISTORY_ITEMS = 18;
        private const int MAX_DEVICES_TO_DISPLAY_ON_DASHBOARD = 200;

        private static readonly TimeSpan CautionAlertMaxDelta = TimeSpan.FromMinutes(91.0);
        private static readonly TimeSpan CriticalAlertMaxDelta = TimeSpan.FromMinutes(11.0);

        private readonly IAlertsLogic _alertsLogic;
        private readonly IDeviceLogic _deviceLogic;
        private readonly IDeviceTelemetryLogic _deviceTelemetryLogic;
        private readonly IConfigurationProvider _configProvider;

        /// <summary>
        /// Initializes a new instance of the TelemetryApiController class.
        /// </summary>
        /// <param name="deviceTelemetryLogic">
        /// The IDeviceRegistryListLogic implementation that the new instance
        /// will use.
        /// </param>
        /// <param name="alertsLogic">
        /// The IAlertsLogic implementation that the new instance will use.
        /// </param>
        /// <param name="deviceLogic">
        /// The IDeviceLogic implementation that the new instance will use.
        /// </param>
        public TelemetryApiController(
            IDeviceTelemetryLogic deviceTelemetryLogic,
            IAlertsLogic alertsLogic,
            IDeviceLogic deviceLogic,
            IConfigurationProvider configProvider)
        {
            if (deviceTelemetryLogic == null)
            {
                throw new ArgumentNullException("deviceTelemetryLogic");
            }

            if (alertsLogic == null)
            {
                throw new ArgumentNullException("alertsLogic");
            }

            if (deviceLogic == null)
            {
                throw new ArgumentNullException("deviceLogic");
            }

            if (configProvider == null)
            {
                throw new ArgumentNullException("configProvider");
            }

            _deviceTelemetryLogic = deviceTelemetryLogic;
            _alertsLogic = alertsLogic;
            _deviceLogic = deviceLogic;
            _configProvider = configProvider;
        }

        [HttpGet]
        [Route("dashboardDevicePane")]
        [WebApiRequirePermission(Permission.ViewTelemetry)]
        public async Task<HttpResponseMessage> GetDashboardDevicePaneDataAsync(string deviceId)
        {
            ValidateArgumentNotNullOrWhitespace("deviceId", deviceId);

            DashboardDevicePaneDataModel result = new DashboardDevicePaneDataModel()
            {
                DeviceId = deviceId
            };

            Func<Task<DashboardDevicePaneDataModel>> getTelemetry =
                async () =>
                {
                    DeviceModel device = await _deviceLogic.GetDeviceAsync(deviceId);

                    IList<DeviceTelemetryFieldModel> telemetryFields = null;

                    try
                    {
                        telemetryFields = _deviceLogic.ExtractTelemetry(device);
                        result.DeviceTelemetryFields = telemetryFields != null ?
                        telemetryFields.ToArray() : null;
                    }
                    catch
                    {
                        HttpResponseMessage message = new HttpResponseMessage(System.Net.HttpStatusCode.InternalServerError);
                        message.Content = new StringContent(
                        string.Format(Strings.InvalidDeviceTelemetryFormat, deviceId));
                        throw new HttpResponseException(message);
                    }

                    // Get Telemetry Summary
                    DeviceTelemetrySummaryModel summaryModel;

                    result.DeviceTelemetrySummaryModel = summaryModel =
                        await _deviceTelemetryLogic.LoadLatestDeviceTelemetrySummaryAsync(
                            deviceId, DateTime.Now.AddMinutes(-MAX_DEVICE_SUMMARY_AGE_MINUTES));

                    if (summaryModel == null)
                    {
                        result.DeviceTelemetrySummaryModel =
                            new DeviceTelemetrySummaryModel();
                    }

                    // Get Telemetry History
                    IEnumerable<DeviceTelemetryModel> telemetryModels;
                    DateTime minTime = DateTime.Now.AddMinutes(-MAX_DEVICE_SUMMARY_AGE_MINUTES);
                    telemetryModels = await _deviceTelemetryLogic.LoadLatestDeviceTelemetryAsync(deviceId, telemetryFields, minTime);

                    if (telemetryModels == null)
                    {
                        result.DeviceTelemetryModels = new DeviceTelemetryModel[0];
                    }
                    else
                    {
                        result.DeviceTelemetryModels =
                            telemetryModels.OrderBy(t => t.Timestamp).ToArray();
                    }

                    return result;
                };

            return await GetServiceResponseAsync<DashboardDevicePaneDataModel>(
                getTelemetry,
                false);
        }

        [HttpGet]
        [Route("list")]
        [WebApiRequirePermission(Permission.ViewTelemetry)]
        public async Task<HttpResponseMessage> GetDeviceTelemetryAsync(
            string deviceId,
            DateTime minTime)
        {
            ValidateArgumentNotNullOrWhitespace("deviceId", deviceId);

            Func<Task<DeviceTelemetryModel[]>> getTelemetry =
                async () =>
                {
                    DeviceModel device = await _deviceLogic.GetDeviceAsync(deviceId);

                    IList<DeviceTelemetryFieldModel> telemetryFields = null;

                    try
                    {
                        telemetryFields = _deviceLogic.ExtractTelemetry(device);
                    }
                    catch
                    {
                        HttpResponseMessage message = new HttpResponseMessage(System.Net.HttpStatusCode.InternalServerError);
                        message.Content = new StringContent(
                            string.Format(Strings.InvalidDeviceTelemetryFormat, deviceId));
                        throw new HttpResponseException(message);
                    }

                    IEnumerable<DeviceTelemetryModel> telemetryModels =
                        await _deviceTelemetryLogic.LoadLatestDeviceTelemetryAsync(
                            deviceId,
                            telemetryFields,
                            minTime);

                    if (telemetryModels == null)
                    {
                        return new DeviceTelemetryModel[0];
                    }

                    return telemetryModels.OrderBy(t => t.Timestamp).ToArray();
                };

            return await GetServiceResponseAsync<DeviceTelemetryModel[]>(
                getTelemetry,
                false);
        }

        [HttpGet]
        [Route("summary")]
        [WebApiRequirePermission(Permission.ViewTelemetry)]
        public async Task<HttpResponseMessage> GetDeviceTelemetrySummaryAsync(string deviceId)
        {
            ValidateArgumentNotNullOrWhitespace("deviceId", deviceId);

            Func<Task<DeviceTelemetrySummaryModel>> getTelemetrySummary =
                async () =>
                {
                    return await _deviceTelemetryLogic.LoadLatestDeviceTelemetrySummaryAsync(
                        deviceId,
                        null);
                };

            return await GetServiceResponseAsync<DeviceTelemetrySummaryModel>(
                getTelemetrySummary,
                false);
        }

        [HttpGet]
        [Route("alertHistory")]
        [WebApiRequirePermission(Permission.ViewTelemetry)]
        public async Task<HttpResponseMessage> GetLatestAlertHistoryAsync()
        {
            Func<Task<AlertHistoryResultsModel>> loadHistoryItems =
                async () =>
                {
                    // Dates are stored internally as UTC and marked as such.  
                    // When parsed, they'll be made relative to the server's 
                    // time zone.  This is only in an issue on servers machines, 
                    // not set to GMT.
                    DateTime currentTime = DateTime.Now;

                    var historyItems = new List<AlertHistoryItemModel>();
                    var deviceModels = new List<AlertHistoryDeviceModel>();
                    var resultsModel = new AlertHistoryResultsModel();

                    IEnumerable<AlertHistoryItemModel> data =
                        await _alertsLogic.LoadLatestAlertHistoryAsync(
                            currentTime.Subtract(CautionAlertMaxDelta),
                            DISPLAYED_HISTORY_ITEMS);

                    if (data != null)
                    {
                        historyItems.AddRange(data);
                        //get alert history
                        List<DeviceModel> devices = await this.LoadAllDevicesAsync();

                        if (devices != null)
                        {
                            Func<string, DateTime?> getStatusTime =
                                _deviceTelemetryLogic.ProduceGetLatestDeviceAlertTime(historyItems);

                            foreach (var device in devices)
                            {
                                if ((device == null) || string.IsNullOrWhiteSpace(device.DeviceProperties.DeviceID))
                                {
                                    continue;
                                }

                                var deviceModel = new AlertHistoryDeviceModel()
                                {
                                    DeviceId = device.DeviceProperties.DeviceID
                                };

                                DateTime? lastStatusTime = getStatusTime(device.DeviceProperties.DeviceID);
                                if (lastStatusTime.HasValue)
                                {
                                    TimeSpan deltaTime = currentTime - lastStatusTime.Value;

                                    if (deltaTime < CriticalAlertMaxDelta)
                                    {
                                        deviceModel.Status = AlertHistoryDeviceStatus.Critical;
                                    }
                                    else if (deltaTime < CautionAlertMaxDelta)
                                    {
                                        deviceModel.Status = AlertHistoryDeviceStatus.Caution;
                                    }
                                }

                                deviceModels.Add(deviceModel);
                            }
                        }
                    }

                    resultsModel.Data = historyItems.Take(DISPLAYED_HISTORY_ITEMS).ToList();
                    resultsModel.Devices = deviceModels;
                    resultsModel.TotalAlertCount = historyItems.Count;
                    resultsModel.TotalFilteredCount = historyItems.Count;

                    return resultsModel;
                };

            return await GetServiceResponseAsync<AlertHistoryResultsModel>(loadHistoryItems, false);
        }

        [HttpGet]
        [Route("currentWeather")]
        [WebApiRequirePermission(Permission.ViewTelemetry)]
        public async Task<HttpResponseMessage> GetCurrentWeatherAsync()
        {
            Func<Task<CurrentWeatherResultsModel>> loadCurrentWeather =
            async () =>
            {
                StringBuilder theWebAddress = new StringBuilder();
                theWebAddress.Append("http://query.yahooapis.com/v1/public/yql?");
                theWebAddress.Append("q=" + HttpUtility.UrlEncode("select * from weather.forecast where woeid in (select woeid from geo.places(1) where text='north greenwich')"));
                theWebAddress.Append("&format=json");
                theWebAddress.Append("&diagnostics=false");

                string results = "";

                using (WebClient wc = new WebClient())
                {
                    results = wc.DownloadString(theWebAddress.ToString());
                }

                WeatherModel weatherModel = Deserialize<WeatherModel>(results);

                CurrentWeatherModel currentWeather = new CurrentWeatherModel()
                {
                    Weather = weatherModel.query.results.channel.item.condition.text,
                    Temperature = weatherModel.query.results.channel.item.condition.temp + weatherModel.query.results.channel.units.temperature,
                    WindSpeed = weatherModel.query.results.channel.wind.speed + weatherModel.query.results.channel.units.speed
                };

                var resultsModel = new CurrentWeatherResultsModel();
                resultsModel.Data = new List<CurrentWeatherModel>();
                resultsModel.Data.Add(currentWeather);

                return resultsModel;
            };

            return await GetServiceResponseAsync<CurrentWeatherResultsModel>(loadCurrentWeather, false);
        }

        [HttpGet]
        [Route("multiDayForecast")]
        [WebApiRequirePermission(Permission.ViewTelemetry)]
        public async Task<HttpResponseMessage> GetMultiDayForecastAsync()
        {
            Func<Task<DailyForecastResultsModel>> loadDailyForecast =
            async () =>
            {
                StringBuilder theWebAddress = new StringBuilder();
                theWebAddress.Append("http://query.yahooapis.com/v1/public/yql?");
                theWebAddress.Append("q=" + HttpUtility.UrlEncode("select * from weather.forecast where woeid in (select woeid from geo.places(1) where text='north greenwich')"));
                theWebAddress.Append("&format=json");
                theWebAddress.Append("&diagnostics=false");

                string results = "";

                using (WebClient wc = new WebClient())
                {
                    results = wc.DownloadString(theWebAddress.ToString());
                }

                WeatherModel weatherModel = Deserialize<WeatherModel>(results);

                var resultsModel = new DailyForecastResultsModel();
                resultsModel.Data = new List<DailyForecastModel>();

                foreach (var forecast in weatherModel.query.results.channel.item.forecast)
                {
                    resultsModel.Data.Add(new DailyForecastModel() { Date = forecast.date, Weather = forecast.text });
                }

                return resultsModel;
            };

            return await GetServiceResponseAsync<DailyForecastResultsModel>(loadDailyForecast, false);
        }

        private async Task<List<DeviceModel>> LoadAllDevicesAsync()
        {
            var query = new DeviceListQuery()
            {
                Skip = 0,
                Take = MAX_DEVICES_TO_DISPLAY_ON_DASHBOARD,
                SortColumn = "DeviceID"
            };

            string deviceId;
            var devices = new List<DeviceModel>();
            DeviceListQueryResult queryResult = await _deviceLogic.GetDevices(query);


            if ((queryResult != null) && (queryResult.Results != null))
            {
                bool? enabledState;
                DeviceProperties props;
                foreach (var devInfo in queryResult.Results)
                {
                    try
                    {
                        deviceId = devInfo.DeviceProperties.DeviceID;
                        props = devInfo.DeviceProperties;
                        enabledState = props.HubEnabledState;
                    }
                    catch (NullReferenceException)
                    {
                        continue;
                    }

                    if (!string.IsNullOrWhiteSpace(deviceId))
                    {
                        devices.Add(devInfo);
                    }
                }
            }

            return devices;
        }

        [HttpGet]
        [Route("mapApiKey")]
        [WebApiRequirePermission(Permission.ViewTelemetry)]
        public async Task<HttpResponseMessage> GetMapApiKey()
        {
            return await GetServiceResponseAsync<string>(async () =>
            {
                String keySetting = await Task.Run(() =>
                {
                    // Set key to empty if passed value 0 from arm template
                    string key = _configProvider.GetConfigurationSettingValue("MapApiQueryKey");
                    return key.Equals("0") ? string.Empty : key;
                });
                return keySetting;
            }, false);
        }

        public static string Serialize<T>(T obj)
        {
            DataContractJsonSerializer serializer = new DataContractJsonSerializer(obj.GetType());
            MemoryStream ms = new MemoryStream();
            serializer.WriteObject(ms, obj);
            string retVal = Encoding.UTF8.GetString(ms.ToArray());
            return retVal;
        }

        public static T Deserialize<T>(string json)
        {
            T obj = Activator.CreateInstance<T>();
            MemoryStream ms = new MemoryStream(Encoding.Unicode.GetBytes(json));
            DataContractJsonSerializer serializer = new DataContractJsonSerializer(obj.GetType());
            obj = (T)serializer.ReadObject(ms);
            ms.Close();
            return obj;
        }
    }
}