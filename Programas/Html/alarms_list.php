<?php
$TITLE='Salidas de Alarma'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='alarms_list_back_btn' class='back-btn' onclick="window.location.replace('alarm_list.php');" >
	<img id='alarms_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='alarms_list_add_btn' class='abm-add-btn' onclick="window.location.replace('alarms_add.php');" >
	<img id='alarms_list_add__icon' class='icon-btn' src='images/add.png'>&nbsp;Nuevo
</div>

<div id='alarms_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAlarmZoneList(JSON.parse(msg).response, 'alarms_list_table_div', '<?php echo $TITLE; ?>', 'Id', 'alarms_edit.php', 'alarms_delete.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=list_salida&Particion=<?php echo $_GET['Id']; ?>', LoadData, false);
    }
</script>

</body>

<?php
include('foot.php');
?>
