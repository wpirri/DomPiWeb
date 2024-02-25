<?php
$TITLE='Borrar Dispositivo'; 
include('head-abm.php');
?>

<body onload="OnLoad();">

<div id='hw_delete_back_btn' class='back-btn' onclick="window.location.replace('hw_list.php');" >
	<img id='hw_delete_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='hw_delete_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='hw_delete_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Borrar
</div>

<div id='hw_delete_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillHWDelete(JSON.parse(msg).response, 'hw_delete_div', '<?php echo $TITLE; ?>');
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmhw.cgi?funcion=delete&Id=<?php echo $_GET['Id']; ?>', null, false);
        window.location.replace('hw_list.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmhw.cgi?funcion=get&Id=<?php echo $_GET['Id']; ?>', LoadData, false);
    }
</script>

</form>

</body>

<?php
include('foot.php');
?>
