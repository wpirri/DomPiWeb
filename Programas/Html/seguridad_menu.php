<?php
$TITLE='Seguridad'; 
include("head.php");
?>

<body>

<div class="back-btn" id="back-from-config" onclick="window.location.replace('<?php echo $MAIN?>');">
	<img id="back-icon" class="icon-btn" src="images/back.png">&nbsp;Volver
</div>



<div class="normal-btn" id="btn-seguridad-camara" onclick="window.location.replace('panel_camara.php');">
	<img id="user-icon" class="icon-btn" src="images/camara.png">&nbsp;C&aacute;maras
</div>

<div class="normal-btn" id="btn-seguridad-alarma" onclick="window.location.replace('panel_alarma.php');">
	<img id="upload-icon" class="icon-btn" src="images/lock1.png">&nbsp;Alarma
</div>
<!--
<div class="normal-btn" id="btn-config-planta" onclick="window.location.replace('planta_edit.php');">
	<img id="home-icon" class="icon-btn" src="images/home.png">&nbsp;Planta
</div>
-->

<!--
<div class="normal-btn" id="btn-config-hard" onclick="window.location.replace('hw_list.php');">
	<img id="disp-icon" class="icon-btn" src="images/hard.png">&nbsp;Dispositivos
</div>
-->
<div class="normal-btn" id="btn-seguridad-panico" onclick="window.location.replace('');">
	<img id="net-icon" class="icon-btn" src="images/panic.png">&nbsp;P&aacute;nico
</div>

<div class="normal-btn" id="btn-seguridad-incendio" onclick="window.location.replace('');">
	<img id="group-icon" class="icon-btn" src="images/reset.png">&nbsp;Incendio
</div>

<div class="normal-btn" id="btn-seguridad-emer-med" onclick="window.location.replace('');">
	<img id="event-icon" class="icon-btn" src="images/people.png">&nbsp;Emergencia M&eacute;dica
</div>

<!--
<div class="normal-btn" id="btn-config-flags" onclick="window.location.replace('flag_list.php');">
	<img id="flags-icon" class="icon-btn" src="images/var.png">&nbsp;Variables
</div>
<div class="normal-btn" id="btn-config-dummy3" onclick="window.location.replace('');">
	<img id="crono-icon" class="icon-btn" src="images/cron.png">&nbsp;Programas
</div>

<div class="normal-btn" id="btn-config-alarm" onclick="window.location.replace('alarm_list.php');">
	<img id="alarm-icon" class="icon-btn" src="images/lock1.png">&nbsp;Alarma
</div>

<div class="normal-btn" id="btn-config-cameras" onclick="window.location.replace('camara_list.php');">
	<img id="camera-icon" class="icon-btn" src="images/camara.png">&nbsp;C&aacute;maras
</div>
-->

<!--
<div class="normal-btn" id="btn-config-upload" onclick="window.location.replace('update_form.php');">
	<img id="upload-icon" class="icon-btn" src="images/upload.png">&nbsp;Actualizaci&oacute;n
</div>

<div class="normal-btn" id="btn-config-download" onclick="window.location.replace('download_list.php');">
	<img id="logs-icon" class="icon-btn" src="images/download.png">&nbsp;Descargas
</div>

<div class="normal-btn" id="btn-config-dummy2" onclick="window.location.replace('');">
	<img id="logs-icon" class="icon-btn" src="images/download.png">&nbsp;xxxxxxxx
</div>

<div class="normal-btn" id="btn-config-auto" onclick="window.location.replace('auto_menu.php');">
	<img id="auto-icon" class="icon-btn" src="images/gear.png">&nbsp;Automatización
</div>
-->

</body>
<?php
include("foot.php");
?>


