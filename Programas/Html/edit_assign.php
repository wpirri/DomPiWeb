<?php
$TITLE='Modificar Objeto'; 
include('head-abm.php');
?>

<body onload="OnLoad();">

<form id="edit_form" name="edit_form" method="post">

<div id='ass_edit_back_btn' class='back-btn' onclick="window.location.replace('planta_edit.php');" >
	<img id='ass_edit_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='ass_edit_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='ass_edit_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='ass_edit_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAssEdit(JSON.parse(msg).response, 'ass_edit_div', '<?php echo $TITLE; ?>');
    }

    function LoadHWData(msg) {
        loadHWTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmassign.cgi?funcion=get&Id=<?php echo $_GET['Id']; ?>', LoadData, false);
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmassign.cgi?funcion=update', null, false, collectFormData('edit_form'));

        setTimeout( "window.location.replace('planta_edit.php')", 1000);
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmhw.cgi', LoadHWData, false);
    }
</script>

</form>

</body>

<?php
include('foot.php');
?>
