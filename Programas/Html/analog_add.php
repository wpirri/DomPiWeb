<?php
$TITLE='Iluminación Nueva'; 
include('head-abm.php');
?>

<body  onload="OnLoad();">

<form id="add_form" name="add_form" method="post">

<div id='analog_add_back_btn' class='back-btn' onclick="window.location.replace('analog_list.php');" >
	<img id='analog_add_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='analog_add_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='analog_add_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='analog_add_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAnalogForm(JSON.parse(msg).response, 'analog_add_div', '<?php echo $TITLE; ?>');
    }

    function LoadAssData(msg) {
        loadAssTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmgroup.cgi', LoadGrpData, false);
    }

    function LoadGrpData(msg) {
        loadGrpTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmauto.cgi?funcion=get&Id=0', LoadData, false);
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmauto.cgi?funcion=add', null, false, collectFormData('add_form'));

        window.location.replace('analog_list.php');
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
