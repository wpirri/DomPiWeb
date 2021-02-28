<?php
$TITLE='Borrar Usuario'; 
include('head-abm.php');
?>

<body onload="GetUser();">

<div id='user_delete_back_btn' class='back-btn' onclick="window.location.replace('user_list.php');" >
	<img id='user_delete_back_icon' class='icon-btn' src='/images/no.png'>&nbsp;Cancelar
</div>

<div id='user_delete_save_btn' class='submit-btn' onclick="DeleteUser();" >
	<img id='user_delete_save_icon' class='icon-btn' src='/images/ok.png'>&nbsp;Borrar
</div>

<p class="abm-title">&nbsp; <?php echo $TITLE; ?> </p>
<div id='user_delete_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadUserData(msg) {
        fillAbmDelete(JSON.parse(msg).usuario, 'user_delete_div');
    }

    function DeleteUser() {
        newAJAXCommand('/cgi-bin/abmuser.cgi?funcion=delete&user_id=<?php echo $_GET['user_id']; ?>', null, false);
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
