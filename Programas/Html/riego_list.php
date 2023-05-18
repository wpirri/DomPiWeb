<?php
$TITLE='Circuito de Riego'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='riego_list_back_btn' class='back-btn' onclick="window.location.replace('<?php echo $AUTO_MENU?>');" >
	<img id='riego_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='riego_list_add_btn' class='abm-add-btn' onclick="window.location.replace('riego_add.php');" >
	<img id='riego_list_add__icon' class='icon-btn' src='images/add.png'>&nbsp;Nuevo
</div>

<div id='riego_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillRiegoList(JSON.parse(msg).response, 'riego_list_table_div', '<?php echo $TITLE; ?>', 'Id', 'riego_edit.php', 'riego_delete.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmauto.cgi?funcion=list&Tipo=1', LoadData, false);
    }
</script>

</body>

<?php
include('foot.php');
?>
