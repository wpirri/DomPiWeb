<?php
$TITLE='Usuarios'; 
include('head.php');
?>

<body onload='GetUserList();'>

<div id='user_list_back_btn' class='abm-back-btn' onclick="window.location.replace('/');" >
	<img id='user_list_back_icon' class='btn-icon' src='/images/back.png'>&nbsp;Volver
</div>

<div id='user_list_add_btn' class='abm-add-btn' onclick="window.location.replace('/');" >
	<img id='user_list_add__icon' class='btn-icon' src='/images/add.png'>&nbsp;Nuevo
</div>

<div id='user_list_table_div' class='abm-list-div'></div>

<script type="text/javascript" >
    function LoadUserList(msg) {
        fillAbmTable(JSON.parse(msg).usuarios, 'user_list_table_div', 'user_id', 'user_edit.php', 'user_delete.php');
    }

    function GetUserList() {
        newAJAXCommand('/cgi-bin/abmuser.cgi', LoadUserList, false);
    }
</script>

</body>

<?php
include('foot.php');
?>
