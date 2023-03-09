<?php
$TITLE='Editar Circuito de Riego'; 
include('head-abm.php');
?>

<body onload="OnLoad();">

<form id="edit_form" name="edit_form" method="post">

<div id='riego_edit_back_btn' class='back-btn' onclick="window.location.replace('riego_list.php');" >
	<img id='riego_edit_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='riego_edit_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='riego_edit_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='riego_edit_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAnalogEdit(JSON.parse(msg).response, 'riego_edit_div', '<?php echo $TITLE; ?>');
    }

    function LoadAssData(msg) {
        loadAssTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmgroup.cgi', LoadGrpData, false);
    }

    function LoadGrpData(msg) {
        loadGrpTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmauto.cgi?funcion=get&Id=<?php echo $_GET['Id']; ?>', LoadData, false);
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmauto.cgi?funcion=update', null, false, collectFormData('edit_form'));

        window.location.replace('riego_list.php');
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
