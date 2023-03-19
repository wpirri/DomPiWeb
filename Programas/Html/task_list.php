<?php
$TITLE='Tareas'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='task_list_back_btn' class='back-btn' onclick="window.location.replace('<?php echo $AUTO_MENU?>');" >
	<img id='task_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='task_list_add_btn' class='abm-add-btn' onclick="window.location.replace('task_add.php');" >
	<img id='task_list_add__icon' class='icon-btn' src='images/add.png'>&nbsp;Nuevo
</div>

<div id='task_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillTaskList(JSON.parse(msg).response, 'task_list_table_div', '<?php echo $TITLE; ?>', 'Id', 'task_edit.php', 'task_delete.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmat.cgi', LoadData, false);
    }
</script>

</body>

<?php
include('foot.php');
?>
