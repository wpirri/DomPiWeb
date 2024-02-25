<?php
$TITLE='Editar Grupo'; 
include('head-abm.php');
?>

<body onload="OnLoad();">

<form id="edit_form" name="edit_form" method="post">

<div id='group_edit_back_btn' class='back-btn' onclick="window.location.replace('group_list.php');" >
	<img id='group_edit_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='group_edit_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='group_edit_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='group_edit_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillGroupEdit(JSON.parse(msg).response, 'group_edit_div', '<?php echo $TITLE; ?>');
    }

    function LoadAssData(msg) {
        loadAssTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmgroup.cgi?funcion=get&Id=<?php echo $_GET['Id']; ?>', LoadData, false);
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmgroup.cgi?funcion=update', null, false, collectFormData('edit_form'));

        window.location.replace('group_list.php');
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
