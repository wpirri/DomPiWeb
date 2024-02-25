<?php
$TITLE='Descargas'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='download_list_back_btn' class='back-btn' onclick="window.location.replace('<?php echo $CONFIG_MENU?>');" >
	<img id='download_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='download_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadList(msg) {

    }

    function OnLoad() {

    }
</script>

</body>

<?php
include('foot.php');
?>
