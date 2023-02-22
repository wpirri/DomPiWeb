<?php
$TITLE='Objeto Nuevo'; 
include('head-abm.php');
?>

<body  onload="OnLoad();">

<form id="add_form" name="add_form" method="post">

<div id='ass_add_back_btn' class='back-btn' onclick="window.location.replace('ass_list.php');" >
	<img id='ass_add_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='ass_add_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='ass_add_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='ass_add_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAssForm(JSON.parse(msg).response, 'ass_add_div', '<?php echo $TITLE; ?>');
    }

    function LoadHWData(msg) {
        loadHWTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmassign.cgi?funcion=get&Id=0', LoadData, false);
    }

    function SaveData() {
        /* Send form data to /cgi-bin/abmuser.cgi?funcion=update */

        var kvpairs = [];
        var form = document.getElementById('add_form');

        for ( var i = 0; i < form.elements.length; i++ ) {
            var e = form.elements[i];
            kvpairs.push(encodeURIComponent(e.name) + '=' + encodeURIComponent(e.value));
        }

        newAJAXCommand('/cgi-bin/abmassign.cgi?funcion=add', null, false, kvpairs.join('&'));

        window.location.replace('ass_list.php');
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
