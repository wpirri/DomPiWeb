<?php
$TITLE='Tactil Nuevo'; 
include('head-abm.php');
?>

<body  onload="OnLoad();">

<form id="add_form" name="add_form" method="post">

<div id='touch_add_back_btn' class='back-btn' onclick="window.location.replace('touch_list.php');" >
	<img id='touch_add_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='touch_add_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='touch_add_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='touch_add_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmForm(JSON.parse(msg).response, 'touch_add_div', '<?php echo $TITLE; ?>');
    }

    function LoadHWData(msg) {
        loadHWTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmtouch.cgi?funcion=get&Id=0', LoadData, false);
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmtouch.cgi?funcion=add', null, false, collectFormData('add_form'));

        window.location.replace('touch_list.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmhw.cgi', LoadHWData, false);
    }
</script>

</form>

</body>

<?php
include('foot.php');
?>
