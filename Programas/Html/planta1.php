<?php
$TITLE='Planta 1'; 
include("head.php");
?>

<body>

<div class="back-btn" id="back-from-planta" onclick="window.location.replace('/');">
	<img id="back-icon" class="icon-btn" src="images/back.png">&nbsp;Volver
</div>

<div class="normal-btn" id="btn-planta-config" onclick="window.location.replace('config.php');">
	<img id="config-icon" class="icon-btn" src="images/system.png">&nbsp;Configuración
</div>

<div class="normal-btn" id="btn-planta-task" onclick="window.location.replace('task.php');">
	<img id="config-icon" class="icon-btn" src="images/task.png">&nbsp;Tareas</div>

<div class="normal-btn" id="btn-planta-alarm" onclick="window.location.replace('alarm.php');">
	<img id="config-icon" class="icon-btn" src="images/lock0.png">&nbsp;Alarma
</div>

</body>
<?php
include("foot.php");
?>


