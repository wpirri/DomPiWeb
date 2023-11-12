<?php
$TITLE='Borrar Camara'; 
include('head-abm.php');
?>

<body onload="OnLoad();">

<div id='camara_delete_back_btn' class='back-btn' onclick="window.location.replace('camara_list.php');" >
	<img id='camara_delete_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='camara_delete_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='camara_delete_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Borrar
</div>

<div id='camara_delete_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmDelete(JSON.parse(msg).response, 'camara_delete_div', '<?php echo $TITLE; ?>');
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmcamara.cgi?funcion=delete&Id=<?php echo $_GET['Id']; ?>', null, false);
        window.location.replace('camara_list.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmcamara.cgi?funcion=get&Id=<?php echo $_GET['Id']; ?>', LoadData, false);
    }
</script>

</body>

<?php
include('foot.php');
?>
