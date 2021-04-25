<!DOCTYPE html>
<meta charset="utf-8">
<html>
<head>
<title>SYSHOME V 3.0</title>
<meta name="author" content="Walter Pirri" >
<meta name="keywords" content="SMART HOME, SYSHOME, DOMOTIC, SECURITY SYSTEM">
<meta name="description" content="Sistema integrado de monitoreo, alarma y domotica">
<meta name="system-build" content="2021">
<link href="/css/index.css" rel="stylesheet" type="text/css" />
<script src="/js/weather.js" type="text/javascript"></script>
</head>
<body>

<div id="reloj">reloj</div>
<div id="info_container">
<table width="100%" border="0"><tr>
  <td align="center"><div id="calendario" class="calendario">calendario</div></td>
  <td align="center"><div id="clima_exterior" class="clima">exterior</div></td>
  <td align="center"><div id="clima_interior" class="clima">interior</div></td>
</tr></table>
</div>
<script type="text/javascript">
/* Clima */
function setCurrentWeather( current ) {
    //document.getElementById('temp_actual').innerHTML = 'Temperatura actual ' + Weather.kelvinToCelsius(current.temperature()).toFixed(1) + ' °C';
    document.getElementById('clima_exterior').innerHTML =
        '<br /><img class "weather-icon" src="/images/w/' + current.data.weather[0].icon + '.png" />' +
        '<br />T ' + Weather.kelvinToCelsius(current.data.main.temp).toFixed(1) + ' °C' + 
        '<br />Hr ' + current.data.main.humidity + ' %' +
        '<br />P ' + current.data.main.pressure + ' hPA' + 
        '<br /><br />' + current.data.weather[0].description;
    
    
}

function setForecasttWeather( forecast ) {
    //document.getElementById('temp_max').innerHTML = 'Temperatura Maxima ' + Weather.kelvinToCelsius(forecast.high()).toFixed(1) + ' °C';
    //document.getElementById('temp_min').innerHTML = 'Temperatura Minima ' + Weather.kelvinToCelsius(forecast.low()).toFixed(1) + ' °C';
}

function setLocalData( local ) {
    //document.getElementById('temp_actual').innerHTML = 'Temperatura actual ' + Weather.kelvinToCelsius(current.temperature()).toFixed(1) + ' °C';
    document.getElementById('clima_interior').innerHTML =
        '<br /><img class "weather-icon" src="/images/home.png" />' +
        '<br />T 0,0 °C' + 
        '<br />Hr 0 %';
}

// API Key methods
var apiKey = '0b39d49d1a75d80d40e7f32edf2cc7c1';
Weather.setApiKey( apiKey );
var tempApiKey = Weather.getApiKey();
// Language methods
var langugage = "es";
Weather.setLanguage( langugage );
var tempLanguage = Weather.getLanguage();
// Get current weather for a given city
Weather.getCurrent( 'Buenos Aires', setCurrentWeather );
// Get the forecast for a given city
Weather.getForecast( 'Buenos Aires', setForecasttWeather );
// Get Local data
setLocalData();
/* Reloj */
function twoDigits(i) {
  if (i < 10) {
    i = "0" + i;
  }
  return i;
}
function setCurrentTime( ) {
    var months = ["Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre"];
    var day_of_week = ["Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"];
    var today = new Date();
    var h = today.getHours();
    var m = today.getMinutes();
    var s = today.getSeconds();
    var yy = today.getFullYear();
    var mm = today.getMonth();
    var dd = today.getDate();
    var dw = today.getDay();
    // add a zero in front of numbers<10
    m = twoDigits(m);
    s = twoDigits(s);
    if(s & 1)
    {
      document.getElementById('reloj').innerHTML = h + ":" + m;// + ":" + s;
    }
    else
    {
      document.getElementById('reloj').innerHTML = h + " " + m;// + ":" + s;
    }

    document.getElementById('calendario').innerHTML = 
              '<div id="dia_semana">' + day_of_week[dw] + '</div>' +
              '<br /><div id="fecha">' + dd + '</div>' +
              '<br /><div id="mes">' + months[mm] + '</div>';

    setTimeout("setCurrentTime()", 500);
}
setCurrentTime();

/* On Click de la página */
window.onclick = function() {window.location.replace('config.php');}

/* Reload para actualizaciones */
setTimeout("window.location.replace('/');", 600000);

</script>
</body>
</html>
