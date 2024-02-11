<?php
$TITLE='Mensaje Infrarrojo Nuevo'; 
include('head-abm.php');
?>

<body  onload="OnLoad();">

<form id="add_form" name="add_form" method="post">

<div id='msgir_add_back_btn' class='back-btn' onclick="window.location.replace('msgir_list.php');" >
	<img id='msgir_add_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='msgir_add_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='msgir_add_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='msgir_add_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmForm(JSON.parse(msg).response, 'msgir_add_div', '<?php echo $TITLE; ?>');
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmmsgir.cgi?funcion=add', null, false, collectFormData('add_form'));

        window.location.replace('msgir_list.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmmsgir.cgi?funcion=get&Id=0', LoadData, false);
    }
</script>

</form>

</body>

<?php
include('foot.php');
?>
