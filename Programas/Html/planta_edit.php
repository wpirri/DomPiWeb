<?php
$TITLE='Plano de Planta'; 
include("head.php");
?>

<body onload="OnLoad();">

<?php
$PLANTA=1;
include("obj_style_edit.php");
?>

<div class="back-btn" id="back-from-planta" onclick="window.location.replace('<?php echo $CONFIG_MENU?>');">
	<img id="back-icon" class="icon-btn" src="images/back.png">&nbsp;Volver
</div>

<div class="normal-btn" id="btn-planta-add" onclick="window.location.replace('planta_add_assign.php');">
	<img id="config-icon" class="icon-btn" src="images/add.png">&nbsp;Nuevo
</div>

<div class="home-group" id="home-div">
<?php
include("obj_draw_edit.php");
?>
	<img class="home-image" id="plano1" src="images/home.png" />
</div>

<script type="text/javascript" >

function updateHomePicture(msg) {
	jsonData = JSON.parse(msg).response;
	var bg_data = jsonData[0].Planta1.split(';');
	document.getElementById('plano1').src = 'images/' + bg_data[0];
	if(bg_data[1] != null && bg_data[2] != null)
	{
		setTimeout("ScrollMap(" + bg_data[1] + "," + bg_data[2] + ");", 1000);
	}
}

function ScrollMap(x, y) {
	map = document.getElementById('home-div');
	map.scrollLeft = x;
	map.scrollTop = y;
}

function OnLoad( ) {
	newAJAXCommand('/cgi-bin/abmsys.cgi?funcion=get_current', updateHomePicture, false);
}

</script>

</body>
<?php
include("foot.php");
?>


