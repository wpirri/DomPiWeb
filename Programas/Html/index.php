<?php 
    include('config.php'); 
?>
<!DOCTYPE html>
<meta charset="utf-8">
<html>
<head>
<title>SYSHOME V 3.0</title>
<meta name="author" content="Walter Pirri" >
<meta name="keywords" content="SMART HOME, SYSHOME, DOMOTIC, SECURITY SYSTEM">
<meta name="description" content="Sistema integrado de monitoreo, alarma y domotica">
<meta name="system-build" content="2021">
<link href="css/index.css" rel="stylesheet" type="text/css" />
<script src="js/weather.js" type="text/javascript"></script>
<script src="js/ajax.js" type="text/javascript"></script>
</head>

<?php
// Detectar Mobile
$mobile_browser = '0';

if( isset($_SERVER['HTTP_ACCEPT']) )
{
  if((strpos(strtolower($_SERVER['HTTP_ACCEPT']),'application/vnd.wap.xhtml+xml')>0) or
    ((isset($_SERVER['HTTP_X_WAP_PROFILE']) or isset($_SERVER['HTTP_PROFILE']))))
  {
    $mobile_browser++;
  }
}

if( isset($_SERVER['HTTP_USER_AGENT']) )
{
  if(preg_match('/(up.browser|up.link|mmp|symbian|smartphone|midp|wap|phone|mobile)/i',strtolower($_SERVER['HTTP_USER_AGENT'])))
  {
    $mobile_browser++;
  }

  if(strpos(strtolower($_SERVER['HTTP_USER_AGENT']),'windows') > 0)
  {
    error_log("MATCH: windows",0);
    $mobile_browser = 0;
  }

}

error_log("DOMPIWEB INDEX - HTTP_ACCEPT:".json_encode($_SERVER['HTTP_ACCEPT']).
          " - HTTP_USER_AGENT:".json_encode($_SERVER['HTTP_USER_AGENT']), 0);

if($mobile_browser > 0)
{
// Mostrar contenido para dispositivos móviles
?>
<script type="text/javascript">
  window.location.replace('m/');
</script>
<?php
}
else
{
// Mostrar contenido para general
?>
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
        '<br /><img class "weather-icon" src="images/w/' + current.data.weather[0].icon + '.png" />' +
        '<br />T ' + Weather.kelvinToCelsius(current.data.main.temp).toFixed(1) + ' °C' + 
        '<br />Hr ' + current.data.main.humidity + ' %' +
        '<br />P ' + current.data.main.pressure + ' hPA' + 
        '<br /><br />' + current.data.weather[0].description;
    
    
}

function setForecasttWeather( forecast ) {
    //document.getElementById('temp_max').innerHTML = 'Temperatura Maxima ' + Weather.kelvinToCelsius(forecast.high()).toFixed(1) + ' °C';
    //document.getElementById('temp_min').innerHTML = 'Temperatura Minima ' + Weather.kelvinToCelsius(forecast.low()).toFixed(1) + ' °C';
}

function updateLocalStatus(msg) {
  var Temp = '0.00';
  var Hum = '0.00';
	jsonData = JSON.parse(msg).response;
	for (var i = 0; i < jsonData.length; i++) { 
		//jsonData[i].Objeto
		//jsonData[i].Port
		//jsonData[i].Icono_Apagado
		//jsonData[i].Icono_Encendido
		//jsonData[i].Estado
		//jsonData[i].Tipo
		//jsonData[i].Perif_Data
		if ( jsonData[i].Tipo == 6 ) {
			if(jsonData[i].Objeto == 'Temp Int') {
				Temp = jsonData[i].Perif_Data;
			} else if(jsonData[i].Objeto == 'Hum Int') {
				Hum = jsonData[i].Perif_Data;
			}
    }
	}
  document.getElementById('clima_interior').innerHTML =
        '<br /><img class "weather-icon" src="images/home.png" />' +
        '<br />T '+ Temp +' °C' + 
        '<br />Hr '+ Hum +' %';
}

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

    if(++timer_counter > 120)
    {
      timer_counter = 0;
      // Get Local data
      newAJAXCommand('/cgi-bin/abmassign.cgi?funcion=status&Planta=1', updateLocalStatus, false);
    }

    setTimeout("setCurrentTime()", 500);
  }

var timer_counter = 118;
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

setCurrentTime();

/* On Click de la página */
window.onclick = function() {window.location.replace('<?php echo $MAIN?>');}

/* Reload para actualizaciones */
setTimeout("window.location.replace('<?php echo $INDEX?>');", 3600000);

</script>
</body>
<?php
}
?>

</html>
