<?php
$TITLE='Editar Usuario'; 
include('head-abm.php');
?>

<body onload="GetUser();">

<form id="edit_form" name="edit_form" method="post">

<div id='user_edit_back_btn' class='back-btn' onclick="window.location.replace('user_list.php');" >
	<img id='user_edit_back_icon' class='icon-btn' src='/images/no.png'>&nbsp;Cancelar
</div>

<div id='user_edit_save_btn' class='submit-btn' onclick="SaveUserData();" >
	<img id='user_edit_save_icon' class='icon-btn' src='/images/ok.png'>&nbsp;Guardar
</div>

<p class="abm-title">&nbsp; <?php echo $TITLE; ?> </p>
<div id='user_edit_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadUserData(msg) {
        fillAbmEdit(JSON.parse(msg).usuario, 'user_edit_div');
    }

    function SaveUserData() {
        /* Send form data to /cgi-bin/abmuser.cgi?funcion=update */

        var kvpairs = [];
        var form = document.getElementById('edit_form');

        for ( var i = 0; i < form.elements.length; i++ ) {
            var e = form.elements[i];
            kvpairs.push(encodeURIComponent(e.name) + '=' + encodeURIComponent(e.value));
        }

        newAJAXCommand('/cgi-bin/abmuser.cgi?funcion=update&' + kvpairs.join('&'), null, false);

        window.location.replace('user_list.php');
    }

    function GetUser() {
        newAJAXCommand('/cgi-bin/abmuser.cgi?funcion=get&user_id=<?php echo $_GET['user_id']; ?>', LoadUserData, false);
    }
</script>

</form>

</body>

<?php
include('foot.php');
?>
