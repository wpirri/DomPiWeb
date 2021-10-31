<?php
$TITLE='Objetos'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='ass_list_back_btn' class='back-btn' onclick="window.location.replace('config.php');" >
	<img id='ass_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='ass_list_add_btn' class='abm-add-btn' onclick="window.location.replace('ass_add.php');" >
	<img id='ass_list_add__icon' class='icon-btn' src='images/add.png'>&nbsp;Nuevo
</div>

<div id='ass_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmList(JSON.parse(msg).response, 'ass_list_table_div', '<?php echo $TITLE; ?>', 'Id', 'ass_edit.php', 'ass_delete.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmassign.cgi', LoadData, false);
    }
</script>

</body>

<?php
include('foot.php');
?>
