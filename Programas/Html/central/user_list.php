<!DOCTYPE html>
<meta charset="utf-8">
<html>
<head>
<title>SYSHOME V 3.0 - Usuarios</title>
<link href="/css/gmh.css" rel="stylesheet" type="text/css" />
<script src="/js/dompiweb.js" type="text/javascript"></script>
<script src="/js/jquery.min.js" type="text/javascript"></script>
</head>
<body onload='GetUserList();'>

<div id='user_list'><table id="user_list_tbl" border="1"></table></div>

<script type="text/javascript" >
function LoadUserList(msg) {
    constructTable(JSON.parse(msg).usuarios, '#user_list_tbl');
}

function GetUserList() {
    newAJAXCommand('/cgi-bin/abmuser.cgi', LoadUserList, false);
}
</script>

</body>
</html>