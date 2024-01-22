<?php
$TITLE='Zonas de Alarma'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='alarmz_list_back_btn' class='back-btn' onclick="window.location.replace('alarm_list.php');" >
	<img id='alarmz_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='alarmz_list_add_btn' class='add-btn' onclick="window.location.replace('alarmz_add.php?Particion=<?php echo $_GET['Id']; ?>');" >
	<img id='alarmz_list_add__icon' class='icon-btn' src='images/add.png'>&nbsp;Nuevo
</div>

<div id='alarmz_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAlarmZoneList(JSON.parse(msg).response, 'alarmz_list_table_div', '<?php echo $TITLE; ?>', 'Id', 'alarmz_edit.php', 'alarmz_delete.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=list_zona&Particion=<?php echo $_GET['Id']; ?>', LoadData, false);
    }
</script>

</body>

<?php
include('foot.php');
?>
