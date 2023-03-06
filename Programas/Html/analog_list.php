<?php
$TITLE='Iluminación'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='analog_list_back_btn' class='back-btn' onclick="window.location.replace('auto_menu.php');" >
	<img id='analog_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='analog_list_add_btn' class='abm-add-btn' onclick="window.location.replace('analog_add.php');" >
	<img id='analog_list_add__icon' class='icon-btn' src='images/add.png'>&nbsp;Nuevo
</div>

<div id='analog_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmList(JSON.parse(msg).response, 'analog_list_table_div', '<?php echo $TITLE; ?>', 'Id', 'analog_edit.php', 'analog_delete.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmauto.cgi?funcion=list&Tipo=4', LoadData, false);
    }
</script>

</body>

<?php
include('foot.php');
?>
