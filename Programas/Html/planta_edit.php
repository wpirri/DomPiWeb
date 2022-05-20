<?php
$TITLE='Plano de Planta'; 
include("head.php");
?>

<body onload="OnLoad();">

<?php
$PLANTA=1;
include("obj_style.php");
?>

<div class="back-btn" id="back-from-planta" onclick="window.location.replace('config.php');">
	<img id="back-icon" class="icon-btn" src="images/back.png">&nbsp;Volver
</div>

<div class="normal-btn" id="btn-planta-add" onclick="window.location.replace('add_assign.php');">
	<img id="config-icon" class="icon-btn" src="images/add.png">&nbsp;Nuevo
</div>

<div class="home-group" id="home-div">
<?php
$edit = 1;
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

function EditObject( name )
{
	alert('Edit: ' + name);
}

function OnLoad( ) {
	ScrollMap(1000,0);
}

</script>

</body>
<?php
include("foot.php");
?>


