<?php
$TITLE='Grupos'; 
include("m_head.php");
?>

<body>
<div class="desktop-group" id="desktop">

<!-- Grupos -->
<div class="menu-btn" id="menu1" onclick="window.location.replace('particiones_m.php');">
	<img id="alarm1-icon" class="icon-image" src="../images/lock0.png" >&nbsp;Alarma
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
<?php
include("m_foot.php");
?>
