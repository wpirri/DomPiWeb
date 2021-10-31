<?php
$TITLE='Configuraci&oacute;n del sistema'; 
include('head-abm.php');
?>

<body onload="OnLoad();">

<form id="edit_form" name="edit_form" method="post">

<div id='sys_config_back_btn' class='back-btn' onclick="window.location.replace('config.php');" >
	<img id='sys_config_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='sys_config_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='sys_config_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='sys_config_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmEdit(JSON.parse(msg).response, 'sys_config_div', '<?php echo $TITLE; ?>');
    }

    function SaveData() {
        /* Send form data to /cgi-bin/abmuser.cgi?funcion=update */

        var kvpairs = [];
        var form = document.getElementById('edit_form');

        for ( var i = 0; i < form.elements.length; i++ ) {
            var e = form.elements[i];
            kvpairs.push(encodeURIComponent(e.name) + '=' + encodeURIComponent(e.value));
        }

        newAJAXCommand('/cgi-bin/abmsys.cgi?funcion=add', null, false, kvpairs.join('&'));

        window.location.replace('config.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmsys.cgi?funcion=get_current', LoadData, false);
    }
</script>

</form>

</body>

<?php
include('foot.php');
?>
