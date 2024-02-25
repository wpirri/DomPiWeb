<?php
$TITLE='Evento Nuevo'; 
include('head-abm.php');
?>

<body  onload="OnLoad();">

<form id="add_form" name="add_form" method="post">

<div id='event_add_back_btn' class='back-btn' onclick="window.location.replace('event_list.php');" >
	<img id='event_add_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='event_add_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='event_add_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='event_add_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillEvForm(JSON.parse(msg).response, 'event_add_div', '<?php echo $TITLE; ?>');
    }

    function LoadAssData(msg) {
        loadAssTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmgroup.cgi', LoadGrpData, false);
    }

    function LoadGrpData(msg) {
        loadGrpTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmev.cgi?funcion=get&Id=0', LoadData, false);
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmev.cgi?funcion=add', null, false, collectFormData('add_form'));

        window.location.replace('event_list.php');
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
