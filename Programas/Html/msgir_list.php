<?php
$TITLE='Mensajes Infrarrojos'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='msgir_list_back_btn' class='back-btn' onclick="window.location.replace('<?php echo $CONFIG_MENU?>');" >
	<img id='msgir_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='msgir_list_add_btn' class='add-btn' onclick="window.location.replace('msgir_add.php');" >
	<img id='msgir_list_add__icon' class='icon-btn' src='images/add.png'>&nbsp;Nuevo
</div>

<div id='msgir_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmList(JSON.parse(msg).response, 'msgir_list_table_div', '<?php echo $TITLE; ?>', 'Id', 'msgir_edit.php', 'msgir_delete.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmmsgir.cgi', LoadData, false);
    }
</script>

</body>

<?php
include('foot.php');
?>
