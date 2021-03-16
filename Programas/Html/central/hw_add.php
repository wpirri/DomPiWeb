<?php
$TITLE='Dispositivo Nuevo'; 
include('head-abm.php');
?>

<body  onload="OnLoad();">

<form id="add_form" name="add_form" method="post">

<div id='hw_add_back_btn' class='back-btn' onclick="window.location.replace('hw_list.php');" >
	<img id='hw_add_back_icon' class='icon-btn' src='/images/no.png'>&nbsp;Cancelar
</div>

<div id='hw_add_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='hw_add_save_icon' class='icon-btn' src='/images/ok.png'>&nbsp;Guardar
</div>

<div id='hw_add_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmForm(JSON.parse(msg).response, 'hw_add_div', '<?php echo $TITLE; ?>');
    }

    function SaveData() {
        /* Send form data to /cgi-bin/abmuser.cgi?funcion=update */

        var kvpairs = [];
        var form = document.getElementById('add_form');

        for ( var i = 0; i < form.elements.length; i++ ) {
            var e = form.elements[i];
            kvpairs.push(encodeURIComponent(e.name) + '=' + encodeURIComponent(e.value));
        }

        newAJAXCommand('/cgi-bin/abmhw.cgi?funcion=add', null, false, kvpairs.join('&'));

        window.location.replace('hw_list.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmhw.cgi?funcion=get&hw_id=000000000000', LoadData, false);
    }
</script>

</form>

</body>

<?php
include('foot.php');
?>
