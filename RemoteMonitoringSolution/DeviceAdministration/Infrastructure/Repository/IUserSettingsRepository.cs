﻿using Microsoft.Azure.Devices.Applications.RemoteMonitoring.Common.Models;
using Microsoft.Azure.Devices.Applications.RemoteMonitoring.DeviceAdmin.Infrastructure.Models;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Azure.Devices.Applications.RemoteMonitoring.DeviceAdmin.Infrastructure.Repository
{
    public interface IUserSettingsRepository
    {
        Task<UserSetting> GetUserSettingValueAsync(string settingKey);
        Task<TableStorageResponse<UserSetting>> SetUserSettingValueAsync(UserSetting setting);
    }
}
