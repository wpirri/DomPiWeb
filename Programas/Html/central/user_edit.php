<?php
$TITLE='Editar Usuario'; 
include('head-abm.php');
?>

<body onload="GetUser( '<?php echo $_GET['user_id']; ?>' );">

<form method="post" action="user_update.php" name="edit">

<div id='user_edit_back_btn' class='abm-back-btn' onclick="window.location.replace('user_list.php');" >
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



        window.location.replace('user_list.php');
    }

    function GetUser(user_id) {
        newAJAXCommand('/cgi-bin/abmuser.cgi?funcion=edit&user_id=' + user_id, LoadUserData, false);
    }
</script>

</form>

</body>

<?php
include('foot.php');
?>
