# Smart plant irrigation system in Arduino

This project reads sensor data such as soil moisture, light levels, temperature, and humidity, then sends the data to ThingsBoard using HTTP over WiFi. To separate sensitive information and simplify configuration, WiFi credentials and ThingsBoard API details are stored in the `secrets.h` file. 🌱📡💡

---

## Using the `secrets.h` File

### Purpose:
The `secrets.h` file is used to store:
- WiFi credentials (SSID and password)
- ThingsBoard API host, access token, and URL

By using `secrets.h`, you can keep sensitive information out of your main code, making the project easier to share or manage. 🔒🛠️📂

### How to Configure:

1. Locate the `secrets.h` file in your project directory. 📂
2. Open the file and replace the placeholder values with your actual credentials. For example: ✍️

```cpp
#ifndef SECRETS_H
#define SECRETS_H

// WiFi credentials
#define WIFI_SSID "YourWiFiSSID"
#define WIFI_PASSWORD "YourWiFiPassword"

// ThingsBoard credentials
#define THINGSBOARD_HOST "https://your-thingsboard-host.com"
#define ACCESS_TOKEN "YourAccessToken"
#define THINGSBOARD_URL "https://your-thingsboard-host.com/api/v1/YourAccessToken/telemetry"

#endif // SECRETS_H
```

3. Save the file. 💾✨

### Important Notes:
- Ensure `secrets.h` is included in the main program file using `#include "secrets.h"`. 📄
- Keep `secrets.h` out of public repositories by adding it to your `.gitignore` file if using version control. 🚫🌍🛡️

---
