<?php
$TITLE='Borrar Grupo de Riego'; 
include('head-abm.php');
?>

<body onload="OnLoad();">

<div id='riego_delete_back_btn' class='back-btn' onclick="window.location.replace('riego_list.php');" >
	<img id='riego_delete_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='riego_delete_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='riego_delete_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Borrar
</div>

<div id='riego_delete_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmDelete(JSON.parse(msg).response, 'riego_delete_div', '<?php echo $TITLE; ?>');
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmauto.cgi?funcion=delete&Id=<?php echo $_GET['Id']; ?>', null, false);
        window.location.replace('riego_list.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmauto.cgi?funcion=get&Id=<?php echo $_GET['Id']; ?>', LoadData, false);
    }
</script>

</form>

</body>

<?php
include('foot.php');
?>
