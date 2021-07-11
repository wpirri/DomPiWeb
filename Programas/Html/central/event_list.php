<?php
$TITLE='Eventos'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='event_list_back_btn' class='back-btn' onclick="window.location.replace('config.php');" >
	<img id='event_list_back_icon' class='icon-btn' src='/images/back.png'>&nbsp;Volver
</div>

<div id='event_list_add_btn' class='abm-add-btn' onclick="window.location.replace('event_add.php');" >
	<img id='event_list_add__icon' class='icon-btn' src='/images/add.png'>&nbsp;Nuevo
</div>

<div id='event_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmList(JSON.parse(msg).response, 'event_list_table_div', '<?php echo $TITLE; ?>', 'Id', 'event_edit.php', 'event_delete.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmev.cgi', LoadData, false);
    }
</script>

</body>

<?php
include('foot.php');
?>
