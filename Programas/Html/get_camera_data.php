<?php
include('http.php');
echo base64_encode( httpRequest('GET', $_GET['src'], $_GET['auth'], '') );
?>
