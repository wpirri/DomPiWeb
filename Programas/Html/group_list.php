<?php
$TITLE='Grupos'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='group_list_back_btn' class='back-btn' onclick="window.location.replace('<?php echo $CONFIG_MENU?>');" >
	<img id='group_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='group_list_add_btn' class='add-btn' onclick="window.location.replace('group_add.php');" >
	<img id='group_list_add__icon' class='icon-btn' src='images/add.png'>&nbsp;Nuevo
</div>

<div id='group_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmList(JSON.parse(msg).response, 'group_list_table_div', '<?php echo $TITLE; ?>', 'Id', 'group_edit.php', 'group_delete.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmgroup.cgi', LoadData, false);
    }
</script>

</body>

<?php
include('foot.php');
?>
