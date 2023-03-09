<?php
$TITLE='Cámaras'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='camera_list_back_btn' class='back-btn' onclick="window.location.replace('<?php echo $CONFIG_MENU?>');" >
	<img id='camera_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='camera_list_add_btn' class='abm-add-btn' onclick="window.location.replace('');" >
	<img id='camera_list_add__icon' class='icon-btn' src='images/add.png'>&nbsp;Nuevo
</div>

<div id='camera_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadList(msg) {

    }

    function OnLoad() {

    }
</script>

</body>

<?php
include('foot.php');
?>
