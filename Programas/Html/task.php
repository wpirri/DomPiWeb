<?php
$TITLE='Tareas'; 
include('head-abm.php');
?>

<body onload='OnLoad;'>

<div id='working_list_back_btn' class='back-btn' onclick="window.location.replace('<?php echo $AUTO_MENU?>');" >
	<img id='working_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='working_list_add_btn' class='abm-add-btn' onclick="window.location.replace('working_add.php');" >
	<img id='working_list_add__icon' class='icon-btn' src='images/add.png'>&nbsp;Nuevo
</div>

<div id='working_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadList(msg) {
        //fillAbmList(JSON.parse(msg).response, 'working_list_table_div', '<?php echo $TITLE; ?>', 'working_id', 'working_edit.php', 'working_delete.php');
    }

    function OnLoad() {
        //newAJAXCommand('/cgi-bin/abmuser.cgi', LoadUserList, false);
    }
</script>

</body>

<?php
include('foot.php');
?>
