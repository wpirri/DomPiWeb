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

<div class="normal-btn" id="btn-planta-cameras" onclick="window.location.replace('panel_camara.php');">
	<img id="auto-icon" class="icon-btn" src="images/camara.png">&nbsp;C&aacute;maras
</div>

<div class="normal-btn" id="btn-planta-config" onclick="window.location.replace('<?php echo $CONFIG_MENU?>');">
	<img id="config-icon" class="icon-btn" src="images/system.png">&nbsp;Configuración
</div>

<div class="normal-btn" id="btn-planta-select" onclick="window.location.replace('panel_auto.php');">
	<img id="config-icon" class="icon-btn" src="images/gear.png">&nbsp;Automatizaci&oacute;n
</div>

<div class="home-group" id="home-div">
<?php
include("obj_draw.php");
?>
	<img class="home-image" id="plano1" src="images/home.png" />
</div>

<script type="text/javascript" >

function updateHomePicture(msg) {
	try {
		jsonData = JSON.parse(msg).response;
		if(jsonData) {
			var bg_data = jsonData[0].Planta1.split(';');
			document.getElementById('plano1').src = 'images/' + bg_data[0];
			if(bg_data[1] != null && bg_data[2] != null)
			{
				setTimeout("ScrollMap(" + bg_data[1] + "," + bg_data[2] + ");", 1000);
			}
		}
		else {
			newAJAXCommand('/cgi-bin/abmsys.cgi?funcion=get_current', updateHomePicture, false);
		}
	} catch (e) { return; }
}

function ScrollMap(x, y) {
	map = document.getElementById('home-div');
	map.scrollLeft = x;
	map.scrollTop = y;
}

function updateHomeStatus(msg) {
	try {
		jsonData = JSON.parse(msg).response;

		if(!jsonData) return;

		for (var i = 0; i < jsonData.length; i++) { 
			//jsonData[i].Objeto
			//jsonData[i].Port
			//jsonData[i].Icono_Apagado
			//jsonData[i].Icono_Encendido
			//jsonData[i].Estado
			//jsonData[i].Tipo
			//jsonData[i].Perif_Data
			var Objeto = jsonData[i].Objeto;
			// Sustituciones (Mantener: obj_style.php objdraw.php planta.php)
			var objId = Objeto.replace(/ /g, '').replace(/\./g, '');
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
				if(jsonData[i].Estado == 0) {
					document.getElementById('id-'+objId).src = 'images/' + jsonData[i].Icono_Apagado;
				} else if(jsonData[i].Estado == 1) {
					document.getElementById('id-'+objId).src = 'images/' + jsonData[i].Icono_Encendido;
				} else if(jsonData[i].Estado == 2) {
					document.getElementById('id-'+objId).src = 'images/' + jsonData[i].Icono_Auto;
				} else {
					document.getElementById('id-'+objId).src = 'images/no.png';
				}
			}
		}
	} catch (e) { return; }
}

function OnLoad( ) {
	newAJAXCommand('/cgi-bin/abmsys.cgi?funcion=get_current', updateHomePicture, false);
	newAJAXCommand('/cgi-bin/abmassign.cgi?funcion=status&Planta=<?php echo $PLANTA; ?>', updateHomeStatus, true);
}

</script>

</body>
<?php
include("foot.php");
?>


