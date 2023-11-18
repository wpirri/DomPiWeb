<?php
$cmd = "ffmpeg -y -rtsp_transport tcp -i ".$_GET['src']." -frames:v 1 -f image2 - 2>/dev/null";
echo base64_encode( shell_exec( $cmd ) );
?>
