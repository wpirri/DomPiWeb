<?php
include("head.php");
?>

<body>

<div class="back-btn" id="cfg0" onclick="window.location.replace('/');">
	<img id="back-icon" class="icon-btn" src="/images/back.png">&nbsp;Inicio
</div>



<div class="normal-btn" id="btn-config-usuarios" onclick="window.location.replace('user_list.php');">
	<img id="user-icon" class="icon-btn" src="/images/access.png">&nbsp;Usuarios
</div>

<div class="normal-btn" id="btn-config-net" onclick="window.location.replace('working_list.php');">
	<img id="net-icon" class="icon-btn" src="/images/network.png">&nbsp;Red
</div>

<div class="normal-btn" id="btn-config-hard" onclick="window.location.replace('hw_list.php');">
	<img id="disp-icon" class="icon-btn" src="/images/hard.png">&nbsp;Dispositivos
</div>


<div class="normal-btn" id="btn-config-groups" onclick="window.location.replace('working_list.php');">
	<img id="group-icon" class="icon-btn" src="/images/group.png">&nbsp;Grupos
</div>

<div class="normal-btn" id="btn-config-events" onclick="window.location.replace('working_list.php');">
	<img id="event-icon" class="icon-btn" src="/images/event.png">&nbsp;Eventos
</div>

<div class="normal-btn" id="btn-config-flags" onclick="window.location.replace('working_list.php');">
	<img id="alarm-icon" class="icon-btn" src="/images/var.png">&nbsp;Variables
</div>

<div class="normal-btn" id="btn-config-alarm" onclick="window.location.replace('working_list.php');">
	<img id="alarm-icon" class="icon-btn" src="/images/lock1.png">&nbsp;Alarma
</div>

<div class="normal-btn" id="btn-config-cameras" onclick="window.location.replace('working_list.php');">
	<img id="camera-icon" class="icon-btn" src="/images/camara.png">&nbsp;C&aacute;maras
</div>

<div class="normal-btn" id="btn-config-crono" onclick="window.location.replace('');">
	<!--<img id="logs-icon" class="icon-btn" src="/images/no.png">&nbsp;-->
</div>

<div class="normal-btn" id="btn-config-upload" onclick="window.location.replace('working_list.php');">
	<img id="upload-icon" class="icon-btn" src="/images/upload.png">&nbsp;Actualizaci&oacute;n
</div>

<div class="normal-btn" id="btn-config-download" onclick="window.location.replace('working_list.php');">
	<img id="logs-icon" class="icon-btn" src="/images/download.png">&nbsp;Descargas
</div>

</div>

</body>
<?php
include("foot.php");
?>

