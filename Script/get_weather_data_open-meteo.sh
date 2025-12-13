#!/bin/sh
# Recuperación de información del clima de Open Meteo
# https://open-meteo.com/
# Genera: {temp:29.0 °C,hum:38 %,pres:1006.5 hPa,icon:sun.png,text:Soleado}
#

DATE=`date +%Y%m%d%H`

TEMP_FILE="/tmp/.temp-weather-open-meteo-${DATE}.json"
OPEN_MATEO_URL="https://api.open-meteo.com/v1/forecast?latitude=-34.5904&longitude=-58.629&daily=weather_code,temperature_2m_max,temperature_2m_min,apparent_temperature_max,apparent_temperature_min,sunrise,sunset,uv_index_max&hourly=temperature_2m,relative_humidity_2m,apparent_temperature,precipitation_probability,rain,weather_code,surface_pressure,wind_speed_10m,wind_direction_10m,visibility&current=temperature_2m,relative_humidity_2m,apparent_temperature,is_day,wind_speed_10m,rain,precipitation,weather_code,surface_pressure"
GET="/usr/bin/curl"
JSON_PARSER="/usr/bin/jq"
WWW_DATA="/var/www/html/data"
WEATHER_FILE="${WWW_DATA}/weather.json"

if [ ! -d "${WWW_DATA}" ]; then
    mkdir -p "${WWW_DATA}"
    chmod 0777 "${WWW_DATA}"
fi

if [ ! -f "${TEMP_FILE}" ]; then
    $GET $OPEN_MATEO_URL > $TEMP_FILE 2>/dev/null
fi

#   WMO - "weather_code": "wmo code"
#   Code	    Description
#   0	            Clear sky
#   1, 2, 3	        Mainly clear, partly cloudy, and overcast
#   45, 48	        Fog and depositing rime fog
#   51, 53, 55	    Drizzle: Light, moderate, and dense intensity
#   56, 57	        Freezing Drizzle: Light and dense intensity
#   61, 63, 65	    Rain: Slight, moderate and heavy intensity
#   66, 67	        Freezing Rain: Light and heavy intensity
#   71, 73, 75	    Snow fall: Slight, moderate, and heavy intensity
#   77	            Snow grains
#   80, 81, 82	    Rain showers: Slight, moderate, and violent
#   85, 86	        Snow showers slight and heavy
#   95 *	        Thunderstorm: Slight or moderate
#   96, 99 *	    Thunderstorm with slight and heavy hail
# (*) Thunderstorm forecast with hail is only available in Central Europe

#  Imagenes
#   sun.png
#   storm.png
#   cloudy.png
#   night.png
#   rainy.png


ICON_CODE=`${JSON_PARSER} '.current.weather_code' "${TEMP_FILE}" | tr -d '\n' | tr -d '\"'`
IS_DAY=`${JSON_PARSER} '.current.is_day' "${TEMP_FILE}" | tr -d '\n' | tr -d '\"'`

case $ICON_CODE in
    # Clear sky
    0)
    if [ "X${IS_DAY}" = "X1" ]; then
        ICON_FILE="sun.png"
        SHOW_TEXT="Soleado"
    else
        ICON_FILE="moon.png"
        SHOW_TEXT="Despejado"
    fi
    ;;
    # Mainly clear, partly cloudy, and overcast
    1|2|3)
    if [ "X${IS_DAY}" = "X1" ]; then
        ICON_FILE="sun-cloud.png"
        SHOW_TEXT="Algo Nublado"
    else
        ICON_FILE="moon-cloud.png"
        SHOW_TEXT="Algo Nublado"
    fi
    ;;
    # Fog and depositing rime fog
    45|48)
    if [ "X${IS_DAY}" = "X1" ]; then
        ICON_FILE="sun-fog.png"
        SHOW_TEXT="Niebla"
    else
        ICON_FILE="moon-fog.png"
        SHOW_TEXT="Niebla"
    fi
    ;;
    # Drizzle: Light, moderate, and dense intensity
    51|53|55)
    if [ "X${IS_DAY}" = "X1" ]; then
        ICON_FILE="sun-rain.png"
        SHOW_TEXT="Llovizna"
    else
        ICON_FILE="moon-rain.png"
        SHOW_TEXT="Llovizna"
    fi
    ;;
    # Freezing Drizzle: Light and dense intensity
    56|57)
    if [ "X${IS_DAY}" = "X1" ]; then
        ICON_FILE="sun-snow.png"
        SHOW_TEXT="Helada"
    else
        ICON_FILE="moon-snow.png"
        SHOW_TEXT="Helada"
    fi
    ;;
    # Rain: Slight, moderate and heavy intensity
    61|63|65)
    if [ "X${IS_DAY}" = "X1" ]; then
        ICON_FILE="sun-rain.png"
        SHOW_TEXT="Llovizna"
    else
        ICON_FILE="moon-rain.png"
        SHOW_TEXT="Llovizna"
    fi
    ;;
    # Freezing Rain: Light and heavy intensity
    66|67)
    if [ "X${IS_DAY}" = "X1" ]; then
        ICON_FILE="sun-rain.png"
        SHOW_TEXT="Lluvia"
    else
        ICON_FILE="moon-rain.png"
        SHOW_TEXT="Lluvia"
    fi
    ;;
    # Snow fall: Slight, moderate, and heavy intensity
    71|73|75)
    if [ "X${IS_DAY}" = "X1" ]; then
        ICON_FILE="sun-snow.png"
        SHOW_TEXT="Nieve"
    else
        ICON_FILE="moon-snow.png"
        SHOW_TEXT="Nieve"
    fi
    ;;
    # Snow grains
    77)
    if [ "X${IS_DAY}" = "X1" ]; then
        ICON_FILE="sun-snow.png"
        SHOW_TEXT="Granizo"
    else
        ICON_FILE="moon-snow.png"
        SHOW_TEXT="Granizo"
    fi
    ;;
    # Rain showers: Slight, moderate, and violent
    80|81|82)
    if [ "X${IS_DAY}" = "X1" ]; then
        ICON_FILE="sun-storm.png"
        SHOW_TEXT="Tormenta"
    else
        ICON_FILE="storm.png"
        SHOW_TEXT="Tormenta"
    fi
    ;;
    # Snow showers slight and heavy
    85|86)
    if [ "X${IS_DAY}" = "X1" ]; then
        ICON_FILE="sun-snow.png"
        SHOW_TEXT="Nevada"
    else
        ICON_FILE="moon-snow.png"
        SHOW_TEXT="Nevada"
    fi
    ;;
    # Thunderstorm: Slight or moderate
    95)
    if [ "X${IS_DAY}" = "X1" ]; then
        ICON_FILE="sun-storm.png"
        SHOW_TEXT="Tormenta"
    else
        ICON_FILE="storm.png"
        SHOW_TEXT="Tormenta"
    fi
    ;;
    # Thunderstorm with slight and heavy hail
    96|99)
    if [ "X${IS_DAY}" = "X1" ]; then
        ICON_FILE="sun-storm.png"
        SHOW_TEXT="Tormenta"
    else
        ICON_FILE="storm.png"
        SHOW_TEXT="Tormenta"
    fi
    ;;
    *)
    if [ "X${IS_DAY}" = "X1" ]; then
        ICON_FILE="sun.png"
        SHOW_TEXT="-"
    else
        ICON_FILE="moon.png"
        SHOW_TEXT="-"
    fi
    ;;
esac

echo -n "{\"response\":{" > "${WEATHER_FILE}"

echo -n "\"temp\":\"" >> "${WEATHER_FILE}"
${JSON_PARSER} '.current.temperature_2m' "${TEMP_FILE}" | tr -d '\n' | tr -d '\"' >> "${WEATHER_FILE}"
echo -n " " >> "${WEATHER_FILE}"
${JSON_PARSER} '.current_units.temperature_2m' "${TEMP_FILE}" | tr -d '\n' | tr -d '\"' >> "${WEATHER_FILE}"
echo -n "\"," >> "${WEATHER_FILE}"

echo -n "\"hum\":\"" >> "${WEATHER_FILE}"
${JSON_PARSER} '.current.relative_humidity_2m' "${TEMP_FILE}" | tr -d '\n' | tr -d '\"' >> "${WEATHER_FILE}"
echo -n " " >> "${WEATHER_FILE}"
${JSON_PARSER} '.current_units.relative_humidity_2m' "${TEMP_FILE}" | tr -d '\n' | tr -d '\"' >> "${WEATHER_FILE}"
echo -n "\"," >> "${WEATHER_FILE}"

echo -n "\"pres\":\"" >> "${WEATHER_FILE}"
${JSON_PARSER} '.current.surface_pressure' "${TEMP_FILE}" | tr -d '\n' | tr -d '\"' >> "${WEATHER_FILE}"
echo -n " " >> "${WEATHER_FILE}"
${JSON_PARSER} '.current_units.surface_pressure' "${TEMP_FILE}" | tr -d '\n' | tr -d '\"' >> "${WEATHER_FILE}"
echo -n "\"," >> "${WEATHER_FILE}"

echo -n "\"icon\":\"${ICON_FILE}\"," >> "${WEATHER_FILE}"
echo -n "\"text\":\"${SHOW_TEXT}\"" >> "${WEATHER_FILE}"

echo "}}" >> "${WEATHER_FILE}"
