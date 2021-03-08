<?php
$TITLE='Usuarios'; 
include('head-abm.php');
?>

<body onload='GetUserList();'>

<div id='user_list_back_btn' class='back-btn' onclick="window.location.replace('config.php');" >
	<img id='user_list_back_icon' class='icon-btn' src='/images/back.png'>&nbsp;Volver
</div>

<div id='user_list_add_btn' class='abm-add-btn' onclick="window.location.replace('user_add.php');" >
	<img id='user_list_add__icon' class='icon-btn' src='/images/add.png'>&nbsp;Nuevo
</div>

<p class="abm-title">&nbsp; <?php echo $TITLE; ?> </p>
<div id='user_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadUserList(msg) {
        fillAbmList(JSON.parse(msg).response, 'user_list_table_div', 'user_id', 'user_edit.php', 'user_delete.php');
    }

    function GetUserList() {
        newAJAXCommand('/cgi-bin/abmuser.cgi', LoadUserList, false);
    }
</script>

</body>

<?php
include('foot.php');
?>
