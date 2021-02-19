<!DOCTYPE html>
<meta charset="utf-8">
<html>
<head>
<title>SYSHOME V 3.0 - Usuarios</title>
<link href="/css/dompiweb.css" rel="stylesheet" type="text/css" />
<script src="/js/dompiweb.js" type="text/javascript"></script>
<script src="/js/abm.js" type="text/javascript"></script>
<script src="/js/jquery.min.js" type="text/javascript"></script>
</head>
<body onload='GetUserList();'>

<div id='user_list_back_btn' class='abm-back-btn' onclick="window.location.replace('/');" >
	<img id='user_list_back_icon' class='btn-icon' src='/images/back.png'>&nbsp;Volver
</div>

<div id='user_list_table_div' class='abm-list-div'></div>

<div id='user_list_add_btn' class='abm-add-btn' onclick="window.location.replace('/');" >
	<img id='user_list_add__icon' class='btn-icon' src='/images/add.png'>&nbsp;Nuevo
</div>

<script type="text/javascript" >
function LoadUserList(msg) {
    fillAbmTable(JSON.parse(msg).usuarios, 'user_list_table_div', 'user_id', 'user_edit.php', 'user_delete.php');
}

function GetUserList() {
    newAJAXCommand('/cgi-bin/abmuser.cgi', LoadUserList, false);
}
</script>

</body>
</html>