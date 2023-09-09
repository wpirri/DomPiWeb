<?php
$TITLE='Tablero de alarmas'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='tablero_list_back_btn' class='back-btn' onclick="window.location.replace('<?php echo $MAIN?>');" >
	<img id='tablero_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='tablero_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    function OnLoad() {

    }
</script>

</body>

<?php
include('foot.php');
?>
