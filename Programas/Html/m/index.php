<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="utf-8">
<title>Grupos</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta name="author" content="Walter Pirri" >
<meta name="keywords" content="SMART HOME, SYSHOME, DOMOTIC, SECURITY SYSTEM, IOT">
<meta name="description" content="Sistema integrado de monitoreo, alarma y domotica">
<meta name="system-build" content="2023">
<link href="../css/movil.css" rel="stylesheet" type="text/css" />
</head>

<body>
<div class="desktop-group" id="desktop">

<!-- Grupos -->
<div class="menu-btn" id="menu1" onclick="window.location.replace('objetos_m.php?grupo=1');">
	<img id="alarm1-icon" class="icon-image" src="../images/lock.png" >&nbsp;Alarma
	<div class="status-text" id="alarm1_status_arm">&nbsp;* ????</div>
	<div class="status-text" id="alarm1_status_rdy">&nbsp;* ????</div>
</div>

<div class="menu-btn" id="menu2" onclick="window.location.replace('objetos_m.php?grupo=2');">
	<img id="home-icon" class="icon-image" src="../images/lamp1.png">&nbsp;Luces
</div>

<div class="menu-btn" id="menu3" onclick="window.location.replace('objetos_m.php?grupo=3');">
	<img id="zone-icon" class="icon-image" src="../images/door1.png">&nbsp;Puertas
</div>

<div class="menu-btn" id="menu4" onclick="window.location.replace('objetos_m.php?grupo=4');">
	<img id="zone-icon" class="icon-image" src="../images/calef1.png">&nbsp;Clima
</div>

<div class="menu-btn" id="menu5" onclick="window.location.replace('objetos_m.php?grupo=5');">
	<img id="zone-icon" class="icon-image" src="../images/camara.png">&nbsp;C&aacute;maras
</div>

<div class="menu-btn" id="menu6" onclick="window.location.replace('objetos_m.php?grupo=6');">
	<img id="zone-icon" class="icon-image" src="../images/gota.png">&nbsp;Riego
</div>


</div>

</body>
</html>
