<?php
$TITLE='Usuario Nuevo'; 
include('head-abm.php');
?>

<body  onload="GetUserStruct();">

<form id="add_form" name="add_form" method="post">

<div id='user_add_back_btn' class='back-btn' onclick="window.location.replace('user_list.php');" >
	<img id='user_add_back_icon' class='icon-btn' src='/images/no.png'>&nbsp;Cancelar
</div>

<div id='user_add_save_btn' class='submit-btn' onclick="SaveUserData();" >
	<img id='user_add_save_icon' class='icon-btn' src='/images/ok.png'>&nbsp;Guardar
</div>

<p class="abm-title">&nbsp; <?php echo $TITLE; ?> </p>
<div id='user_add_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadUserForm(msg) {
        fillAbmForm(JSON.parse(msg).usuario, 'user_add_div');
    }

    function SaveUserData() {
        /* Send form data to /cgi-bin/abmuser.cgi?funcion=update */

        var kvpairs = [];
        var form = document.getElementById('add_form');

        for ( var i = 0; i < form.elements.length; i++ ) {
            var e = form.elements[i];
            kvpairs.push(encodeURIComponent(e.name) + '=' + encodeURIComponent(e.value));
        }

        newAJAXCommand('/cgi-bin/abmuser.cgi?funcion=add&' + kvpairs.join('&'), null, false);

        window.location.replace('user_list.php');
    }

    function GetUserStruct() {
        newAJAXCommand('/cgi-bin/abmuser.cgi?funcion=get&user_id=admin', LoadUserForm, false);
    }
</script>

</form>

</body>

<?php
include('foot.php');
?>
