<?php
$TITLE='Camara Nueva'; 
include('head-abm.php');
?>

<body  onload="OnLoad();">

<form id="add_form" name="add_form" method="post">

<div id='camara_add_back_btn' class='back-btn' onclick="window.location.replace('camara_list.php');" >
	<img id='camara_add_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='camara_add_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='camara_add_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='camara_add_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmForm(JSON.parse(msg).response, 'camara_add_div', '<?php echo $TITLE; ?>');
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmcamara.cgi?funcion=add', null, false, collectFormData('add_form'));

        window.location.replace('camara_list.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmcamara.cgi?funcion=get&Id=0', LoadData, false);
    }
</script>

</form>

</body>

<?php
include('foot.php');
?>
