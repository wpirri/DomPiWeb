<?php
$TITLE='Borrar Zona de Alarma'; 
include('head-abm.php');
?>

<body onload="OnLoad();">

<div id='alarmz_delete_back_btn' class='back-btn' onclick="window.location.replace('alarm_list.php');" >
	<img id='alarmz_delete_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='alarmz_delete_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='alarmz_delete_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Borrar
</div>

<div id='alarmz_delete_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmDelete(JSON.parse(msg).response, 'alarmz_delete_div', '<?php echo $TITLE; ?>');
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=delete_zona&Id=<?php echo $_GET['Id']; ?>', null, false);
        window.location.replace('alarm_list.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=get_zona&Id=<?php echo $_GET['Id']; ?>', LoadData, false);
    }
</script>

</form>

</body>

<?php
include('foot.php');
?>
