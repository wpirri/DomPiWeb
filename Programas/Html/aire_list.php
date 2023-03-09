<?php
$TITLE='Aire Acondicionado'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='riego_list_back_btn' class='back-btn' onclick="window.location.replace('<?php echo $AUTO_MENU?>');" >
	<img id='riego_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='riego_list_add_btn' class='abm-add-btn' onclick="window.location.replace('');" >
	<img id='riego_list_add__icon' class='icon-btn' src='images/add.png'>&nbsp;Nuevo
</div>

<div id='riego_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {

    }

    function OnLoad() {

    }
</script>

</body>

<?php
include('foot.php');
?>
