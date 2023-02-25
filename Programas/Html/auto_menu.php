<?php
$TITLE='Automatizaciones'; 
include("head.php");
?>

<body>

<div class="back-btn" id="back-from-config" onclick="window.location.replace('<?php echo $MAIN?>');">
	<img id="back-icon" class="icon-btn" src="images/back.png">&nbsp;Volver
</div>



<div class="normal-btn" id="btn-config-riego" onclick="window.location.replace('riego_list.php');">
	<img id="riego-icon" class="icon-btn" src="images/gota.png">&nbsp;Riego
</div>

<div class="normal-btn" id="btn-config-calefaccion" onclick="window.location.replace('calefaccion_list.php');">
	<img id="calefaccion-icon" class="icon-btn" src="images/calef1.png">&nbsp;Calefacci&oacute;n
</div>

<div class="normal-btn" id="btn-config-aire" onclick="window.location.replace('aire_edit.php');">
	<img id="aire-icon" class="icon-btn" src="images/aire.png">&nbsp;Aire
</div>


<div class="normal-btn" id="btn-config-hard" onclick="window.location.replace('task_list.php');">
	<img id="task-icon" class="icon-btn" src="images/cron.png">&nbsp;Tareas
</div>
<!--
<div class="normal-btn" id="btn-config-assign" onclick="window.location.replace('ass_list.php');">
	<img id="net-icon" class="icon-btn" src="images/lamp1.png">&nbsp;Objetos
</div>

<div class="normal-btn" id="btn-config-groups" onclick="window.location.replace('group_list.php');">
	<img id="group-icon" class="icon-btn" src="images/group.png">&nbsp;Grupos
</div>

<div class="normal-btn" id="btn-config-events" onclick="window.location.replace('event_list.php');">
	<img id="event-icon" class="icon-btn" src="images/event.png">&nbsp;Eventos
</div>
-->

<!--
<div class="normal-btn" id="btn-config-flags" onclick="window.location.replace('flag_list.php');">
	<img id="flags-icon" class="icon-btn" src="images/var.png">&nbsp;Variables
</div>

<div class="normal-btn" id="btn-config-crono" onclick="window.location.replace('task.php');">
	<img id="crono-icon" class="icon-btn" src="images/cron.png">&nbsp;Programas
</div>

<div class="normal-btn" id="btn-config-alarm" onclick="window.location.replace('alarm.php');">
	<img id="alarm-icon" class="icon-btn" src="images/lock1.png">&nbsp;Alarma
</div>

<div class="normal-btn" id="btn-config-cameras" onclick="window.location.replace('working_list.php');">
	<img id="camera-icon" class="icon-btn" src="images/camara.png">&nbsp;C&aacute;maras
</div>
-->


<!--
<div class="normal-btn" id="btn-config-upload" onclick="window.location.replace('working_list.php');">
	<img id="upload-icon" class="icon-btn" src="images/upload.png">&nbsp;Actualizaci&oacute;n
</div>

<div class="normal-btn" id="btn-config-download" onclick="window.location.replace('working_list.php');">
	<img id="logs-icon" class="icon-btn" src="images/download.png">&nbsp;Descargas
</div>

<div class="normal-btn" id="btn-config-dummy2" onclick="window.location.replace('working_list.php');">
	<img id="logs-icon" class="icon-btn" src="images/download.png">&nbsp;xxxxxxxx
</div>

<div class="normal-btn" id="btn-config-dummy3" onclick="window.location.replace('working_list.php');">
	<img id="logs-icon" class="icon-btn" src="images/download.png">&nbsp;xxxxxxxx
</div>
-->

</div>

</body>
<?php
include("foot.php");
?>


