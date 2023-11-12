<?php
$TITLE='Camaras'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='camara_list_back_btn' class='back-btn' onclick="window.location.replace('<?php echo $CONFIG_MENU?>');" >
	<img id='camara_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='camara_list_add_btn' class='abm-add-btn' onclick="window.location.replace('camara_add.php');" >
	<img id='camara_list_add__icon' class='icon-btn' src='images/add.png'>&nbsp;Nuevo
</div>

<div id='camara_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmList(JSON.parse(msg).response, 'camara_list_table_div', '<?php echo $TITLE; ?>', 'Id', 'camara_edit.php', 'camara_delete.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmcamara.cgi', LoadData, false);
    }
</script>

</body>

<?php
include('foot.php');
?>
