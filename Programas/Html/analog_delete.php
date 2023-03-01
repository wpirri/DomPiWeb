<?php
$TITLE='Borrar Analogico'; 
include('head-abm.php');
?>

<body onload="OnLoad();">

<div id='analog_delete_back_btn' class='back-btn' onclick="window.location.replace('analog_list.php');" >
	<img id='analog_delete_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='analog_delete_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='analog_delete_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Borrar
</div>

<div id='analog_delete_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmDelete(JSON.parse(msg).response, 'analog_delete_div', '<?php echo $TITLE; ?>');
    }

    function LoadAssData(msg) {
        loadAssTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmauto.cgi?funcion=get&Id=<?php echo $_GET['Id']; ?>', LoadData, false);
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmauto.cgi?funcion=delete&Id=<?php echo $_GET['Id']; ?>', null, false);
        window.location.replace('analog_list.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmassign.cgi', LoadAssData, false);
    }
</script>

</form>

</body>

<?php
include('foot.php');
?>
