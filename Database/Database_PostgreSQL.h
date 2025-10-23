#ifndef DATABASE_POSTGRESQL_H
#define DATABASE_POSTGRESQL_H
#include <stdbool.h>

#define DATABASE_POSTGRESQL_CITY_FIELD_COUNT 20
#define DATABASE_POSTGRESQL_MAX_DATA_SIZE 2048
#define DATABASE_POSTGRESQL_URL "https://oaxzbzejuorwpxtgeduv.supabase.co"
#define DATABASE_POSTGRESQL_TABLE_CITY_WEATHER "/rest/v1/city_weather"
#define DATABASE_POSTGRESQL_API_KEY "api key here"
#define DATABASE_POSTGRESQL_AUTH_HEADER "Authorization: Bearer your-auth-token-here"

typedef struct {
    char* cityname; /* uses same name as City, no need to free */
    double latitude;
    double longitude;
    long long timeLongNumber; /* time as number (huge number)*/
    char* timeISO8601; /* new: text string time from json data. ! allocd char str !*/
    double precipitation; /* new */
    char* precipitationUnits; /* new free after use */
    int temperature; /* °C ?*/
    char* temperatureUnits;  /* allocd char str*/
    int elevation; /* new: elevation data*/
    int weathercode; /* 0-100 */
    char* weathercodeVerbose; /* verbose interpretation of WMO Weather Code*/
    double windspeed;
    char* windspeedUnits; /* allocd char str*/
    double windspeedMS; /* converted to m/s */
    double windspeedKMPH; /* default windspeed */
    int windDirection; /* new  */
    char* windDirectionUnits; /* allocd char string ! */
    char* windDirectionVerbose;
    bool isDay; /* new isDay day = 1 or night = 0 */
} CitySqlData;

int Database_PostgreSQL_Perform_Insert();

#endif