<?php
$TITLE='Particion de Alarma Nueva'; 
include('head-abm.php');
?>

<body  onload="OnLoad();">

<form id="add_form" name="add_form" method="post">

<div id='alarm_add_back_btn' class='back-btn' onclick="window.location.replace('alarm_list.php');" >
	<img id='alarm_add_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='alarm_add_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='alarm_add_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='alarm_add_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadAssData(msg) {
        loadAssTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=get_part&Id=0', LoadData, false);
    }

    function LoadData(msg) {
        fillAlarmForm(JSON.parse(msg).response, 'alarm_add_div', '<?php echo $TITLE; ?>');
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=add_part', null, false, collectFormData('add_form'));

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
