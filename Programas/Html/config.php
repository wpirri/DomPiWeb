<?php
$INDEX='index.html';
$MAIN='planta.php';
$CONFIG_MENU='config_menu.php';
$AUTO_MENU='auto_menu.php';
$UPLOAD_FOLDER='upload';
$DOWNLOAD_FOLDER='download';

$TIMEOUT=900000;

function head_link(string $filename)
{
    if( ($fm = filemtime($filename)) == false )
    {
        $fm = 0;
    }
    echo "<link href=\"".$filename."?time=".$fm."\" rel=\"stylesheet\" type=\"text/css\" />\n";
}

function head_script(string $filename)
{
    if( ($fm = filemtime($filename)) == false )
    {
        $fm = 0;
    }
    echo "<script src=\"".$filename."?time=".$fm."\" type=\"text/javascript\"></script>\n";
}

?>
