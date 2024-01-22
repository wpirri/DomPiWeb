<?php
$TITLE='Usuarios'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='user_list_back_btn' class='back-btn' onclick="window.location.replace('<?php echo $CONFIG_MENU?>');" >
	<img id='user_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='user_list_add_btn' class='add-btn' onclick="window.location.replace('user_add.php');" >
	<img id='user_list_add__icon' class='icon-btn' src='images/add.png'>&nbsp;Nuevo
</div>

<div id='user_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmList(JSON.parse(msg).response, 'user_list_table_div', '<?php echo $TITLE; ?>', 'Id', 'user_edit.php', 'user_delete.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmuser.cgi', LoadData, false);
    }
</script>

</body>

<?php
include('foot.php');
?>
