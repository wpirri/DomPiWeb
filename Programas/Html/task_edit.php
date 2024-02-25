<?php
$TITLE='Editar Tarea'; 
include('head-abm.php');
?>

<body onload="OnLoad();">

<form id="edit_form" name="edit_form" method="post">

<div id='task_edit_back_btn' class='back-btn' onclick="window.location.replace('task_list.php');" >
	<img id='task_edit_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='task_edit_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='task_edit_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='task_edit_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillTaskEdit(JSON.parse(msg).response, 'task_edit_div', '<?php echo $TITLE; ?>');
    }

    function LoadAssData(msg) {
        loadAssTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmgroup.cgi', LoadGrpData, false);
    }

    function LoadGrpData(msg) {
        loadGrpTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmat.cgi?funcion=get&Id=<?php echo $_GET['Id']; ?>', LoadData, false);
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmat.cgi?funcion=update', null, false, collectFormData('edit_form'));

        window.location.replace('task_list.php');
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
