<?php
$TITLE='Borrar Salida de Alarma'; 
include('head-abm.php');
?>

<body onload="OnLoad();">

<div id='alarms_delete_back_btn' class='back-btn' onclick="window.location.replace('alarm_list.php');" >
	<img id='alarms_delete_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='alarms_delete_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='alarms_delete_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Borrar
</div>

<div id='alarms_delete_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmDelete(JSON.parse(msg).response, 'alarms_delete_div', '<?php echo $TITLE; ?>');
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=delete_salida&Id=<?php echo $_GET['Id']; ?>', null, false);
        window.location.replace('alarm_list.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=get_salida&Id=<?php echo $_GET['Id']; ?>', LoadData, false);
    }
</script>

</form>

</body>

<?php
include('foot.php');
?>
