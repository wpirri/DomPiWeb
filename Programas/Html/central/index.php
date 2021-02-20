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

<div id='temp_actual'>temp_actual</div>
<div id='humedad'>humedad</div>
<div id='presion'>humedad</div>

<div id='temp_max'>temp_max</div>
<div id='temp_min'>temp_min</div>



<script type="text/javascript">
/* Clima */
function setCurrentWeather( current ) {
    //document.getElementById('temp_actual').innerHTML = 'Temperatura actual ' + Weather.kelvinToCelsius(current.temperature()).toFixed(1) + ' °C';
    document.getElementById('temp_actual').innerHTML = 'Temperatura actual ' + Weather.kelvinToCelsius(current.data.main.temp).toFixed(1) + ' °C';
    document.getElementById('humedad').innerHTML = 'Humedad ' + current.data.main.humidity + ' %';
    document.getElementById('presion').innerHTML = 'Presion ' + current.data.main.pressure + ' hPA';
}

function setForecasttWeather( forecast ) {
    document.getElementById('temp_max').innerHTML = 'Temperatura Maxima ' + Weather.kelvinToCelsius(forecast.high()).toFixed(1) + ' °C';
    document.getElementById('temp_min').innerHTML = 'Temperatura Minima ' + Weather.kelvinToCelsius(forecast.low()).toFixed(1) + ' °C';
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

/* Reloj */
function twoDigits(i) {
  if (i < 10) {
    i = "0" + i;
  }
  return i;
}
function setCurrentTime( ) {
    var today = new Date();
    var h = today.getHours();
    var m = today.getMinutes();
    var s = today.getSeconds();
    // add a zero in front of numbers<10
    m = twoDigits(m);
    s = twoDigits(s);
    document.getElementById('reloj').innerHTML = h + ":" + m + ":" + s;
    setTimeout("setCurrentTime()", 500);
}
setCurrentTime();

/* On Click de la página */
window.onclick = function() {window.location.replace('test.php');}

/* Reload para actualizaciones */
setTimeout("window.location.replace('/');", 600000);

</script>
</body>
</html>
