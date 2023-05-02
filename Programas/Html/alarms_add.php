<?php
$TITLE='Salida de Alarma Nueva'; 
include('head-abm.php');
?>

<body  onload="OnLoad();">

<form id="add_form" name="add_form" method="post">

<div id='alarms_add_back_btn' class='back-btn' onclick="window.location.replace('alarm_list.php');" >
	<img id='alarms_add_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='alarms_add_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='alarms_add_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='alarms_add_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadAssData(msg) {
        loadAssTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=get_salida&Id=0', LoadData, false);
    }

    function LoadData(msg) {
        fillAlarmForm(JSON.parse(msg).response, 'alarms_add_div', '<?php echo $TITLE; ?>');
        document.getElementById('Particion').value = '<?php echo $_GET['Particion']; ?>';
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=add_salida', null, false, collectFormData('add_form'));

        window.location.replace('alarm_list.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmassign.cgi', LoadAssData, false);
    }
</script>

</form>

</body>

<?php
include('foot.php');
?>
