<?php
session_start();
$_SESSION["auth_token"] = "-";
?>
<!DOCTYPE html>
<meta charset="utf-8">
<html>
<head>
<title>SYSHOME V 3.0 - Login</title>
<link href="/css/dompiweb.css" rel="stylesheet" type="text/css" />
</head>
<body>
<form id=login name=login>

<div id=div-login-dialog>
<p>Nombre</p>
<input type="text" name="user_name" id="user-name" />
<p>Clave</p>
<input type="text" name="user_pass" id="user-pass" />
</div>
<div id="div-login-enter" onclick="GoLogin();">

</div>

</form>
</body>

<script type="text/javascript" >
function GoLogin() {

}
</script>

</html>
