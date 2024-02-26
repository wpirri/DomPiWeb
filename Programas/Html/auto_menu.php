<?php
$TITLE='Automatizaciones'; 
include("head.php");
?>

<body>

<div class="back-btn" id="back-from-config" onclick="window.location.replace('<?php echo $CONFIG_MENU?>');">
	<img id="back-icon" class="icon-btn" src="images/back.png">&nbsp;Volver
</div>



<div class="normal-btn" id="btn-config-riego" onclick="window.location.replace('riego_list.php');">
	<img id="riego-icon" class="icon-btn" src="images/gota.png">&nbsp;Riego
</div>

<!--<div class="normal-btn" id="btn-config-calefaccion" onclick="window.location.replace('calefaccion_list.php');">
	<img id="calefaccion-icon" class="icon-btn" src="images/calef1.png">&nbsp;Calefacci&oacute;n
</div>-->

<!--<div class="normal-btn" id="btn-config-aire" onclick="window.location.replace('aire_list.php');">
	<img id="aire-icon" class="icon-btn" src="images/termost.png">&nbsp;Aire
</div>-->


<div class="normal-btn" id="btn-config-task" onclick="window.location.replace('task_list.php');">
	<img id="task-icon" class="icon-btn" src="images/cron.png">&nbsp;Tareas
</div>

<div class="normal-btn" id="btn-config-analog" onclick="window.location.replace('analog_list.php');">
	<img id="analog-icon" class="icon-btn" src="images/lamp1.png">&nbsp;Iluminaci&oacute;n
</div>

</body>
<?php
include("foot.php");
?>


