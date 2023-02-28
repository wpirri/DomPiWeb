<?php
$TITLE='Grupo de Riego Nuevo'; 
include('head-abm.php');
?>

<body  onload="OnLoad();">

<form id="add_form" name="add_form" method="post">

<div id='riego_add_back_btn' class='back-btn' onclick="window.location.replace('riego_list.php');" >
	<img id='riego_add_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='riego_add_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='riego_add_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='riego_add_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmForm(JSON.parse(msg).response, 'riego_add_div', '<?php echo $TITLE; ?>');
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmauto.cgi?funcion=add&Tipo=1', null, false, collectFormData('add_form'));

        window.location.replace('riego_list.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmauto.cgi?funcion=get&Id=0', LoadData, false);
    }
</script>

</form>

</body>

<?php
include('foot.php');
?>
