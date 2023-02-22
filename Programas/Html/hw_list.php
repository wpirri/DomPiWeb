<?php
$TITLE='Dispositivos'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='hw_list_back_btn' class='back-btn' onclick="window.location.replace('<?php echo $CONFIG_MENU?>');" >
	<img id='hw_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='hw_list_add_btn' class='abm-add-btn' onclick="window.location.replace('hw_add.php');" >
	<img id='hw_list_add__icon' class='icon-btn' src='images/add.png'>&nbsp;Nuevo
</div>

<div id='hw_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillHWList(JSON.parse(msg).response, 'hw_list_table_div', '<?php echo $TITLE; ?>', 'Id', 'hw_edit.php', 'hw_delete.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmhw.cgi', LoadData, false);
    }
</script>

</body>

<?php
include('foot.php');
?>
