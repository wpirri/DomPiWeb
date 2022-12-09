<?php
$TITLE='Plano de Planta'; 
include("head.php");
?>

<body onload="OnLoad();">

<?php
$PLANTA=1;
include("obj_style.php");
?>

<div class="back-btn" id="back-from-planta" onclick="window.location.replace('<?php echo $INDEX?>');">
	<img id="back-icon" class="icon-btn" src="images/back.png">&nbsp;Volver
</div>

<div class="normal-btn" id="btn-planta-config" onclick="window.location.replace('<?php echo $CONFIG_MENU?>');">
	<img id="config-icon" class="icon-btn" src="images/system.png">&nbsp;Configuración
</div>

<div class="normal-btn" id="btn-planta-auto" onclick="window.location.replace('auto_menu.php');">
	<img id="auto-icon" class="icon-btn" src="images/gear.png">&nbsp;Automatización
</div>

<div class="normal-btn" id="btn-planta-alarm" onclick="window.location.replace('alarm.php');">
	<img id="config-icon" class="icon-btn" src="images/lock0.png">&nbsp;Alarma
</div>

<div class="home-group" id="home-div">
<?php
include("obj_draw.php");
?>
	<img class="home-image" id="plano1" src="images/home1.jpg" />
</div>

<script type="text/javascript" >

function ScrollMap(x, y) {
	map = document.getElementById('home-div');
	map.scrollLeft = x;
	map.scrollTop = y;
}

function OnLoad( ) {
	ScrollMap(1000,0);
}

function updateHomeStatus(msg) {
	jsonData = JSON.parse(msg).response;
	for (var i = 0; i < jsonData.length; i++) { 
		//jsonData[i].Objeto
		//jsonData[i].Port
		//jsonData[i].Icono0
		//jsonData[i].Icono1
		//jsonData[i].Estado
		//jsonData[i].Tipo
		//jsonData[i].Perif_Data
		var Objeto = jsonData[i].Objeto;
		var objId = Objeto.replace(/ /g, "-");
		if ( jsonData[i].Tipo == 2 ) {
			document.getElementById('id-'+objId).innerHTML = '&nbsp' + jsonData[i].Estado + '&nbsp';
		} else if ( jsonData[i].Tipo == 6 ) {
			if(jsonData[i].Port[0] == 'T') {
				document.getElementById('id-'+objId).innerHTML = '&nbsp;T ' + jsonData[i].Perif_Data + ' °C&nbsp;';
			} else if(jsonData[i].Port[0] == 'H') {
				document.getElementById('id-'+objId).innerHTML = '&nbsp;Hr ' + jsonData[i].Perif_Data + ' %&nbsp;';
			} else {
				document.getElementById('id-'+objId).innerHTML = '&nbsp;' + jsonData[i].Perif_Data + '&nbsp;';
			}
		} else {
			if(jsonData[i].Estado == 1) {
				document.getElementById('id-'+objId).src = 'images/' + jsonData[i].Icono1;
			} else {
				document.getElementById('id-'+objId).src = 'images/' + jsonData[i].Icono0;
			}
		}
	}
}

function InitUpdate() {
	newAJAXCommand('/cgi-bin/abmassign.cgi?funcion=status&Planta=<?php echo $PLANTA; ?>', updateHomeStatus, true);
}

InitUpdate();

</script>

</body>
<?php
include("foot.php");
?>


