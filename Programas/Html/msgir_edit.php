<?php
$TITLE='Editar Mensaje Infrarrojo'; 
include('head-abm.php');
?>

<body onload="OnLoad();">

<form id="edit_form" name="edit_form" method="post">

<div id='msgir_edit_back_btn' class='back-btn' onclick="window.location.replace('msgir_list.php');" >
	<img id='msgir_edit_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='msgir_edit_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='msgir_edit_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='msgir_edit_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmEdit(JSON.parse(msg).response, 'msgir_edit_div', '<?php echo $TITLE; ?>');
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmmsgir.cgi?funcion=update', null, false, collectFormData('edit_form'));

        window.location.replace('msgir_list.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmmsgir.cgi?funcion=get&Id=<?php echo $_GET['Id']; ?>', LoadData, false);
    }
</script>

</form>

</body>

<?php
include('foot.php');
?>
