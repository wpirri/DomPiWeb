<?php
$TITLE='Editar Usuario'; 
include('head-abm.php');
?>

<body onload="OnLoad();">

<form id="edit_form" name="edit_form" method="post">

<div id='user_edit_back_btn' class='back-btn' onclick="window.location.replace('user_list.php');" >
	<img id='user_edit_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='user_edit_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='user_edit_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='user_edit_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmEdit(JSON.parse(msg).response, 'user_edit_div', '<?php echo $TITLE; ?>');
    }

    function SaveData() {
        /* Send form data to /cgi-bin/abmuser.cgi?funcion=update */

        var kvpairs = [];
        var form = document.getElementById('edit_form');

        for ( var i = 0; i < form.elements.length; i++ ) {
            var e = form.elements[i];
            kvpairs.push(encodeURIComponent(e.name) + '=' + encodeURIComponent(e.value));
        }

        newAJAXCommand('/cgi-bin/abmuser.cgi?funcion=update', null, false, kvpairs.join('&'));

        window.location.replace('user_list.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmuser.cgi?funcion=get&Id=<?php echo $_GET['Id']; ?>', LoadData, false);
    }
</script>

</form>

</body>

<?php
include('foot.php');
?>
