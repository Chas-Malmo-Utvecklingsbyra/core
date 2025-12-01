#ifndef OPENMETEO_H
#define OPENMETEO_H

#define OPENMETEO_API "https://api.open-meteo.com/v1/forecast?latitude=52.52&longitude=13.41&current=temperature_2m"
#define OPENMETEO_API_TEMPLATE "https://api.open-meteo.com/v1/forecast?latitude=%s&longitude=%s&current=temperature_2m,apparent_temperature,wind_speed_10m,wind_direction_10m,relativehumidity_2m,weather_code&daily=sunrise,sunset&forecast_days=1&temperature_unit=celsius&wind_speed_unit=kmh&timezone=auto"

/*
    const char* data = "https://api.open-meteo.com/v1/forecast?latitude=52.52&longitude=13.41&current=temperature_2m,relative_humidity_2m,"
        "apparent_temperature,is_day,precipitation,rain,showers,snowfall,weather_code,cloud_cover,wind_speed_10m,wind_direction_10m,"
        "wind_gusts_10m,surface_pressure,pressure_msl&wind_speed_unit=ms";
*/

#endif