<?php
$TITLE='Particiones de Alarma'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='alarm_list_back_btn' class='back-btn' onclick="window.location.replace('<?php echo $CONFIG_MENU?>');" >
	<img id='alarm_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='alarm_list_add_btn' class='add-btn' onclick="window.location.replace('alarm_add.php');" >
	<img id='alarm_list_add__icon' class='icon-btn' src='images/add.png'>&nbsp;Nuevo
</div>

<div id='alarm_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAlarmPartList(JSON.parse(msg).response, 'alarm_list_table_div', '<?php echo $TITLE; ?>', 'Id', 'alarm_edit.php', 'alarmz_list.php', 'alarms_list.php', 'alarm_delete.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=list_part', LoadData, false);
    }
</script>

</body>

<?php
include('foot.php');
?>
